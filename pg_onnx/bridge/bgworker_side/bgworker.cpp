//
// Created by Kibae Shin on 2023/09/13.
//

#include "bgworker_side.hpp"

#include <transport/tcp/tcp_server.hpp>

int bgworker_init(extension_state_t *state, volatile sig_atomic_t *terminated) {
	// logger init with elog pipe
	AixLog::Log::init<SinkElog>();

	// thread for run io_context
	std::thread server_thread([state, terminated]() {
		onnxruntime_server::config config;
		config.num_threads = state->config_num_threads;

		boost::asio::io_context io_context;
		Orts::onnx::session_manager manager(model_bin_getter, config.num_threads);

		Orts::transport::tcp::tcp_server server(io_context, config, manager);
		state->port = server.port();

		elog(LOG, "onnxruntime-server TCP backend thread ready: port: %d", state->port);
		auto timeout = std::chrono::milliseconds{1000};
		while (!*terminated) {
			io_context.run_for(timeout);
		}
	});

	while (!*terminated) {
		ResetLatch(&MyProc->procLatch);

		/* Wait for a client connection, a signal, or a timeout */
		auto rc =
			WaitLatch(&MyProc->procLatch, WL_LATCH_SET | WL_TIMEOUT | WL_POSTMASTER_DEATH, 100L, PG_WAIT_EXTENSION);

		if (rc & WL_POSTMASTER_DEATH) {
			elog(LOG, "Got latch event: WL_POSTMASTER_DEATH");
			*terminated = true;
		}
	}

	server_thread.join();

	elog(LOG, "onnxruntime-server TCP backend terminated");

	return 0;
}

std::string model_bin_getter(const std::string &model_name, const std::string &model_version) {
	return "";
}
