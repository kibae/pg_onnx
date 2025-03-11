//
// Created by Kibae Shin on 2023/09/13.
//

#include "task.hpp"

#include "pg_srf_macro.h"
#include "session_response_macro.h"

#include <catalog/pg_type.h>
#include <libpq/libpq-fs.h>

class lo_post_data : public post_data {
	Datum _fd = -1;
	size_t offset = 0;

  public:
	lo_post_data(Oid oid) : post_data() {
		auto fd = DatumGetInt32(DirectFunctionCall2(be_lo_open, oid, Int32GetDatum(INV_READ)));
		if (fd < 0)
			throw std::runtime_error("failed to open large object");

		_fd = Int32GetDatum(fd);

		DirectFunctionCall3(be_lo_lseek64, _fd, Int64GetDatum(0), Int32GetDatum(SEEK_END));
		_content_length = DatumGetInt64(DirectFunctionCall1(be_lo_tell64, _fd));
		DirectFunctionCall3(be_lo_lseek64, _fd, Int64GetDatum(0), Int32GetDatum(SEEK_SET));
	};

	~lo_post_data() {
		if (_fd >= 0)
			DirectFunctionCall1(be_lo_close, _fd);
	}

	bool eof() override {
		return offset >= _content_length;
	}

	size_t read(char *buffer, size_t size) override {
		auto data = DatumGetByteaP(DirectFunctionCall2(be_loread, _fd, Int32GetDatum(size)));
		size_t data_size = VARSIZE_ANY_EXHDR(data);

		memcpy(buffer, VARDATA_ANY(data), data_size);
		offset += data_size;
		return data_size;
	}
};

typedef struct model_info_t {
	std::shared_ptr<lo_post_data> bin = nullptr;
	json option;
} model_info_t;

std::shared_ptr<model_info_t> get_model(const std::string &model_name, const std::string &model_version) {
	std::string sql("SELECT lo_oid, option FROM ext_pg_onnx.model WHERE name=$1 AND version=$2");

	int ret = SPI_connect();
	if (ret != SPI_OK_CONNECT) {
		throw std::runtime_error("SPI_connect failed: error code " + std::to_string(ret));
	}

	Oid argtypes[] = {TEXTOID, TEXTOID};
	SPIPlanPtr plan = SPI_prepare(sql.c_str(), 2, argtypes);
	if (plan == nullptr) {
		SPI_finish();
		throw std::runtime_error("SPI_prepare failed");
	}

	Datum values[] = {CStringGetTextDatum(model_name.c_str()), CStringGetTextDatum(model_version.c_str())};
	ret = SPI_execute_plan(plan, values, nullptr, true, 0);
	if (ret != SPI_OK_SELECT) {
		SPI_finish();
		throw std::runtime_error("SPI_execute failed: error code " + std::to_string(ret));
	}

	// check row count
	if (SPI_processed != 1) {
		SPI_finish();
		throw std::runtime_error("model not found");
	}

	bool is_null;
	auto oid_datum = SPI_getbinval(SPI_tuptable->vals[0], SPI_tuptable->tupdesc, 1, &is_null);
	if (is_null) {
		SPI_finish();
		throw std::runtime_error("model not found");
	}

	json option;
	// option can be null.
	auto option_val = SPI_getbinval(SPI_tuptable->vals[0], SPI_tuptable->tupdesc, 2, &is_null);
	if (is_null) {
		option = json::object();
	} else {
		Jsonb *option_datum = DatumGetJsonbP(option_val);
		auto option_data = JsonbToCString(nullptr, &option_datum->root, VARSIZE_ANY_EXHDR(option_datum));

		option = strlen(option_data) > 1 ? json::parse(option_data) : json::object();
		pfree(option_data);
	}

	auto model_info = std::make_shared<model_info_t>();
	model_info->bin = std::make_shared<lo_post_data>(DatumGetObjectId(oid_datum));
	model_info->option = option;

	SPI_finish();
	return model_info;
}

json create_session(extension_state_t *state, const std::string &name, const std::string &version) {
	auto model = get_model(name, version);

	auto request = json::object();
	request["model"] = name;
	request["version"] = version;
	request["option"] = model->option;

	// If onnxruntime-server is running in remote, pass model binary data as post body,
	// if running in local, save to temporary file and pass file path.
	File tmp_file = -1;
	if (!is_onnxruntime_server_remote(state)) {
		tmp_file = OpenTemporaryFile(false);
		std::string buffer;
		buffer.resize(1024 * 1024 * 4);
		off_t offset = 0;
		while (!model->bin->eof()) {
			auto bytes_read = model->bin->read(buffer.data(), buffer.size());
			auto written = FileWrite(
				tmp_file, buffer.data(), bytes_read, offset,
#if PG_VERSION_NUM < 140000
				0
#else
				WAIT_EVENT_DATA_FILE_WRITE
#endif
			);
			if (written <= 0)
				throw std::runtime_error("failed to write temporary file");
			offset += written;
		}

		request["option"]["path"] = FilePathName(tmp_file);
		model->bin = nullptr;
	}

	auto result = api_request(state, Orts::task::type::CREATE_SESSION, request, model->bin);
	if (tmp_file >= 0)
		FileClose(tmp_file);

	if (result.is_object() && result.contains("error"))
		throw std::runtime_error(result["error"].get<std::string>());
	return result;
}

Datum task_create_session(
	extension_state_t *state, PG_FUNCTION_ARGS, const std::string &name, const std::string &version
) {
	PG_SRF_BEGIN() {
		// process
		auto result = create_session(state, name, version);

		funcctx->user_fctx = (void *)new json_list_iterator(json::array({result}));
	}
	PG_SRF_END();

	RESPONSE_SESSION_LIST
}
