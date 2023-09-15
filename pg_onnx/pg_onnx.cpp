#include "pg_onnx.hpp"
#include "bridge/process_side/task/task.hpp"

extern "C" {
PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(pg_onnx_internal_list_session);
PG_FUNCTION_INFO_V1(pg_onnx_internal_create_session);
PG_FUNCTION_INFO_V1(pg_onnx_internal_execute_session);

Datum pg_onnx_internal_list_session(PG_FUNCTION_ARGS) {
	auto state = extension_state();
	Assert(state != NULL);

	try {
		return task_list_session(state, fcinfo);
	} catch (std::exception &e) {
		elog(ERROR, "%s: %s", __FUNCTION__, e.what());
	}
}

Datum pg_onnx_internal_create_session(PG_FUNCTION_ARGS) {
	// check args count and is null
	if (PG_NARGS() < 2 || PG_ARGISNULL(0) || PG_ARGISNULL(1))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("invalid argument")));

	// args: name TEXT, version TEXT
	auto name = std::string(text_to_cstring(PG_GETARG_TEXT_P(0)));
	auto version = std::string(text_to_cstring(PG_GETARG_TEXT_P(1)));
	if (name.empty() || version.empty())
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("invalid argument")));

	auto state = extension_state();
	Assert(state != NULL);

	try {
		return task_create_session(state, fcinfo, name, version);
	} catch (std::exception &e) {
		elog(ERROR, "%s: %s", __FUNCTION__, e.what());
	}
}

Datum pg_onnx_internal_execute_session(PG_FUNCTION_ARGS) {
	// check args count and is null
	if (PG_NARGS() < 3 || PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("invalid argument")));

	// args: name TEXT, version TEXT, inputs jsonb
	auto name = std::string(text_to_cstring(PG_GETARG_TEXT_P(0)));
	auto version = std::string(text_to_cstring(PG_GETARG_TEXT_P(1)));
	Jsonb *inputs_jsonb = PG_GETARG_JSONB_P(2);
	if (name.empty() || version.empty() || !JsonContainerIsObject(&inputs_jsonb->root))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("invalid argument")));

	auto inputs = JsonbToCString(NULL, &inputs_jsonb->root, VARSIZE(inputs_jsonb));

	auto state = extension_state();
	Assert(state != NULL);

	try {
		return task_execute_session(state, fcinfo, name, version, json::parse(inputs));
	} catch (std::exception &e) {
		elog(ERROR, "%s: %s", __FUNCTION__, e.what());
	}
}
}
