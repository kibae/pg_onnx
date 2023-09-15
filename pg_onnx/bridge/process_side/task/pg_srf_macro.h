//
// Created by Kibae Shin on 2023/09/14.
//

#ifndef PG_ONNX_PG_SRF_MACRO_H
#define PG_ONNX_PG_SRF_MACRO_H

class json_list_iterator {
  public:
	int current = 0;
	json list;

	json_list_iterator(json list) : list(list) {
	}

	json next() {
		if (list.size() <= current)
			return nullptr;

		return list[current++];
	}
};

#define PG_SRF_BEGIN()                                                                                                 \
	FuncCallContext *funcctx;                                                                                          \
	TupleDesc tupdesc;                                                                                                 \
	AttInMetadata *attinmeta;                                                                                          \
                                                                                                                       \
	if (SRF_IS_FIRSTCALL()) {                                                                                          \
		MemoryContext oldcontext;                                                                                      \
		funcctx = SRF_FIRSTCALL_INIT();                                                                                \
		oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);                                            \
                                                                                                                       \
		if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE)                                        \
			ereport(                                                                                                   \
				ERROR, (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),                                                        \
						errmsg("function returning record called in context that cannot accept type record"))          \
			);                                                                                                         \
                                                                                                                       \
		attinmeta = TupleDescGetAttInMetadata(tupdesc);                                                                \
		funcctx->attinmeta = attinmeta;

#define PG_SRF_END()                                                                                                   \
	MemoryContextSwitchTo(oldcontext);                                                                                 \
	}                                                                                                                  \
	funcctx = SRF_PERCALL_SETUP();

#endif // PG_ONNX_PG_SRF_MACRO_H
