//
// Created by Kibae Shin on 2023/09/14.
//

#include "pg_onnx.hpp"

#include "bridge/bgworker_side/bgworker_side.hpp"

static volatile sig_atomic_t terminated = false;

void init_config(extension_state_t *state) {
	// TODO: add config for remote standalone worker
	/*
	DefineCustomStringVariable(
		"pg_onnx.worker_host", "onnxruntime-server standalone server hostname",
		"If onnxruntime-server is running remotely as a standalone, set the hostname. If not set, it will run in a "
		"local background worker.",
		&(state->config_onnxruntime_server_hostname), NULL, PGC_SUSET, 0, NULL, NULL, NULL
	);
	 */

	if (strlen(state->config_onnxruntime_server_hostname) <= 0) {
		// local worker
		DefineCustomIntVariable(
			"pg_onnx.num_threads", "number of threads onnxruntime will run in",
			"Sets the number of threads onnxruntime will run in.", &state->config_num_threads, 4, 1, 100, PGC_SUSET, 0,
			NULL, NULL, NULL
		);
	} else {
		/*
		DefineCustomIntVariable(
			"pg_onnx.worker_port", "onnxruntime-server standalone server port",
			"If onnxruntime-server is running remotely as a standalone, set the port.",
			&state->config_onnxruntime_server_port, -1, 1, 1 << 16, PGC_SUSET, 0, NULL, NULL, NULL
		);
		 */
	}
}

// for disable name mangling
extern "C" {
void _PG_init(void) {
	BackgroundWorker worker;
	BgwHandleStatus status;
	auto state = extension_state();

	if (
#if PG_VERSION_NUM >= 130000
		MyBackendType == B_BG_WORKER
#else
		IsBackgroundWorker
#endif
		|| state->handle != NULL) {
		return;
	}

	init_config(state);

	if (is_onnxruntime_server_remote(state))
		return;

	MemSet(&worker, 0, sizeof(BackgroundWorker));
	worker.bgw_flags = BGWORKER_SHMEM_ACCESS | BGWORKER_BACKEND_DATABASE_CONNECTION;
	worker.bgw_start_time = BgWorkerStart_RecoveryFinished;
	snprintf(worker.bgw_name, BGW_MAXLEN, "pg_onnx");
	snprintf(worker.bgw_type, BGW_MAXLEN, "pg_onnx");
	snprintf(worker.bgw_library_name, BGW_MAXLEN, "pg_onnx");
	snprintf(worker.bgw_function_name, BGW_MAXLEN, "worker_main"); // worker_main
	worker.bgw_restart_time = BGW_NEVER_RESTART;
	worker.bgw_notify_pid = MyProcPid; // for WaitForBackgroundWorkerStartup

	if (!RegisterDynamicBackgroundWorker(&worker, &(state->handle))) {
		ereport(
			ERROR, (errcode(ERRCODE_INSUFFICIENT_RESOURCES), errmsg("could not start background process"),
					errhint("More details may be available in the server log."))
		);
		return;
	}

	status = WaitForBackgroundWorkerStartup(state->handle, &(state->pid));
	if (status == BGWH_STOPPED)
		ereport(
			ERROR, (errcode(ERRCODE_INSUFFICIENT_RESOURCES), errmsg("could not start background process"),
					errhint("More details may be available in the server log."))
		);
	else if (status == BGWH_POSTMASTER_DIED)
		ereport(
			ERROR,
			(errcode(ERRCODE_INSUFFICIENT_RESOURCES), errmsg("cannot start background processes without postmaster"),
			 errhint("Kill all remaining database processes and restart the database."))
		);

	Assert(status == BGWH_STARTED);

	int i = 1000;
	while (state->port <= 0 && i-- > 0) {
		elog(LOG, "Waiting for background worker to start");
		pg_usleep(10000L);
	}

	if (state->port <= 0)
		elog(ERROR, "background worker did not start");
}

void _PG_fini(void) {
	auto state = extension_state();

	if (state->handle != NULL) {
		TerminateBackgroundWorker(state->handle);
		WaitForBackgroundWorkerShutdown(state->handle);
		pfree(state->handle);
		state->handle = NULL;
	}
}

static void worker_sigterm(SIGNAL_ARGS) {
	elog(WARNING, "Background worker got SIGTERM");
	int save_errno = errno;
	terminated = true;
	if (MyProc)
		SetLatch(&MyProc->procLatch);
	errno = save_errno;
}

void worker_exit(extension_state_t *state, int code) {
	state->proc = NULL;
	proc_exit(code);
}

void worker_main(Datum main_arg) {
	auto state = extension_state();
	if (state->proc != NULL) {
		elog(ERROR, "Background worker: worker_main called twice");
		return;
	}

	if (!MyProc) {
		elog(ERROR, "Background worker: worker_main called outside of worker");
		return;
	}
	state->proc = MyProc;

	pqsignal(SIGTERM, worker_sigterm);
	pqsignal(SIGPIPE, SIG_IGN);

	BackgroundWorkerUnblockSignals();

	int exit_code = bgworker_init(state, &terminated);

	worker_exit(state, exit_code);
}
}
