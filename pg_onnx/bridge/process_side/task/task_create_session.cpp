//
// Created by Kibae Shin on 2023/09/13.
//

#include "task.hpp"

#include "pg_srf_macro.h"
#include "session_response_macro.h"

typedef struct model_info_t {
	std::vector<unsigned char> bin;
	json option;
} model_info_t;

model_info_t &get_model(const std::string &model_name, const std::string &model_version) {
	std::string sql("SELECT model, option FROM ext_pg_onnx.model WHERE name=$1 AND version=$2");

	int ret = SPI_connect();
	if (ret != SPI_OK_CONNECT) {
		throw std::runtime_error("SPI_connect failed: error code " + std::to_string(ret));
	}

	SPIPlanPtr plan = SPI_prepare(sql.c_str(), 2, (Oid[]){TEXTOID, TEXTOID});
	if (plan == nullptr) {
		SPI_finish();
		throw std::runtime_error("SPI_prepare failed");
	}

	ret = SPI_execute_plan(
		plan, (Datum[]){CStringGetTextDatum(model_name.c_str()), CStringGetTextDatum(model_version.c_str())}, nullptr,
		true, 0
	);
	if (ret != SPI_OK_SELECT) {
		SPI_finish();
		throw std::runtime_error("SPI_execute failed: error code " + std::to_string(ret));
	}

	// check row count
	if (SPI_processed != 1) {
		SPI_finish();
		throw std::runtime_error("model not found");
	}

	static model_info_t model_info;
	model_info.bin.clear();
	model_info.option.clear();

	bool is_null;
	auto binary_datum = SPI_getbinval(SPI_tuptable->vals[0], SPI_tuptable->tupdesc, 1, &is_null);
	if (is_null) {
		SPI_finish();
		throw std::runtime_error("model not found");
	}
	auto binary_data = DatumGetByteaPP(binary_datum);
	model_info.bin.resize(VARSIZE_ANY_EXHDR(binary_data));
	memcpy(model_info.bin.data(), VARDATA(binary_data), VARSIZE_ANY_EXHDR(binary_data));

	// option can be null.
	auto option_datum = SPI_getbinval(SPI_tuptable->vals[0], SPI_tuptable->tupdesc, 2, &is_null);
	if (is_null) {
		model_info.option = json::object();
	} else {
		auto option_data = text_to_cstring(DatumGetTextP(option_datum));
		model_info.option = strlen(option_data) > 1 ? json::parse(option_data) : json::object();
		pfree(option_data);
	}

	SPI_finish();
	return model_info;
}

json create_session(extension_state_t *state, const std::string &name, const std::string &version) {
	auto model = get_model(name, version);

	auto request = json::object();
	request["model"] = name;
	request["version"] = version;
	request["option"] = model.option;

	auto result =
		api_request(state, Orts::task::type::CREATE_SESSION, request, (const char *)model.bin.data(), model.bin.size());
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
