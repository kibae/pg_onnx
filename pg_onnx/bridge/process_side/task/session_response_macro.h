//
// Created by Kibae Shin on 2023/09/14.
//

#ifndef PG_ONNX_SESSION_RESPONSE_MACRO_H
#define PG_ONNX_SESSION_RESPONSE_MACRO_H

#define RESPONSE_SESSION_LIST                                                                                          \
	auto session_list = (json_list_iterator *)funcctx->user_fctx;                                                      \
	auto session = session_list->next();                                                                               \
	if (session == nullptr) {                                                                                          \
		delete session_list;                                                                                           \
		SRF_RETURN_DONE(funcctx);                                                                                      \
	} else {                                                                                                           \
		std::vector<char *> values;                                                                                    \
		HeapTuple tuple;                                                                                               \
		Datum result;                                                                                                  \
                                                                                                                       \
		values.push_back(pstrdup(session["model"].get<std::string>().c_str()));                                        \
		values.push_back(pstrdup(session["version"].get<std::string>().c_str()));                                      \
		values.push_back(pstrdup(std::to_string(session["created_at"].get<int64_t>()).c_str()));                       \
		values.push_back(pstrdup(std::to_string(session["last_executed_at"].get<int64_t>()).c_str()));                 \
		values.push_back(pstrdup(std::to_string(session["execution_count"].get<int64_t>()).c_str()));                  \
		values.push_back(pstrdup(session["inputs"].dump().c_str()));                                                   \
		values.push_back(pstrdup(session["outputs"].dump().c_str()));                                                  \
		values.push_back(pstrdup(session["option"].dump().c_str()));                                                   \
                                                                                                                       \
		tuple = BuildTupleFromCStrings(funcctx->attinmeta, values.data());                                             \
		result = HeapTupleGetDatum(tuple);                                                                             \
                                                                                                                       \
		for (auto &value : values)                                                                                     \
			pfree(value);                                                                                              \
                                                                                                                       \
		SRF_RETURN_NEXT(funcctx, result);                                                                              \
	}

#endif // PG_ONNX_SESSION_RESPONSE_MACRO_H
