//
// Created by Kibae Shin on 2023/09/14.
//

#ifndef PG_ONNX_PG_ONNX_HPP
#define PG_ONNX_PG_ONNX_HPP

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <postgres.h>

#include <fmgr.h>

#include <access/xact.h>
#include <commands/dbcommands.h>
#include <executor/spi.h>
#include <funcapi.h>
#include <lib/stringinfo.h>
#include <libpq/pqformat.h>
#include <miscadmin.h>
#include <postmaster/bgworker.h>
#include <server/pgstat.h>
#include <storage/ipc.h>
#include <storage/latch.h>
#include <storage/proc.h>
#include <storage/procsignal.h>
#include <storage/shmem.h>
#include <utils/builtins.h>
#include <utils/guc.h>
#include <utils/jsonb.h>
#include <utils/lsyscache.h>
#include <utils/snapmgr.h>
#include <utils/wait_event.h>
}

#include "onnxruntime_server.hpp"
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <vector>

#define CONFIG_MAX_LENGTH 512

typedef struct {
	// background worker
	BackgroundWorkerHandle *handle;
	pid_t pid;

	PGPROC *proc;

	// onnxruntime server
	uint_least16_t port;

	// config
	char config_onnxruntime_server_hostname[CONFIG_MAX_LENGTH]; // for remote standalone worker
	int config_onnxruntime_server_port;							// for remote standalone worker
	int config_num_threads;
} extension_state_t;

extension_state_t *extension_state();

#endif // PG_ONNX_PG_ONNX_HPP
