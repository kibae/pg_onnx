# Functions

- [ONNX Model Functions](#onnx-model-functions)
    - [pg_onnx_import_model(TEXT, TEXT, BYTEA, JSONB, TEXT)](#pg_onnx_import_modeltext-text-bytea-jsonb-text)
    - [pg_onnx_drop_model(TEXT, TEXT)](#pg_onnx_drop_modeltext-text)
    - [pg_onnx_list_model()](#pg_onnx_list_model)
    - [pg_onnx_inspect_model_bin(BYTEA, JSONB)](#pg_onnx_inspect_model_bin-bytea-jsonb)
- [ONNX Session Functions](#onnx-session-functions)
    - [pg_onnx_create_session(TEXT, TEXT)](#pg_onnx_create_sessiontext-text)
    - [pg_onnx_describe_session(TEXT, TEXT)](#pg_onnx_describe_sessiontext-text)
    - [pg_onnx_execute_session(TEXT, TEXT, JSONB)](#pg_onnx_execute_sessiontext-text-jsonb)
    - [pg_onnx_destroy_session(TEXT, TEXT, JSONB)](#pg_onnx_destroy_sessiontext-text-jsonb)
    - [pg_onnx_list_session()](#pg_onnx_list_session)

----

## ONNX Model Functions

### pg_onnx_import_model(TEXT, TEXT, BYTEA, JSONB, TEXT)

- Import the ONNX file.
- The `option(JSONB)` is stored on the model and forwarded verbatim to the bundled `onnxruntime-server` when the session is created. See [Model Options](#model-options) below for available keys.
- Parameters
    - `name(TEXT)`: Model name
    - `version(TEXT)`: Model version
    - `model(BYTEA)`: ONNX model binary data
    - `option(JSONB)`: Options. See [Model Options](#model-options).
    - `description(TEXT)`: Model description
- Returns
    - boolean: `true` if the model is successfully imported, `false` otherwise.

```sql
SELECT pg_onnx_import_model(
        'sample_model',
        'v20230101',
        PG_READ_BINARY_FILE('/your_model_path/model.onnx')::bytea,
        '{"cuda": true}'::jsonb,
        'sample model'
    );
```

```
 pg_onnx_import_model 
----------------------
 t
(1 row)
```

#### Model Options

The `option` JSONB is forwarded to `onnxruntime-server` and consumed when the per-model ONNX runtime session is created. The keys below are passed through transparently. See the upstream [onnxruntime-server #116](https://github.com/kibae/onnxruntime-server/pull/116) for the complete grammar.

> Only values that ORT actually applies are echoed back via [`pg_onnx_describe_session`](#pg_onnx_describe_sessiontext-text) and [`pg_onnx_list_session`](#pg_onnx_list_session). Keys rejected by ORT silently drop out of the echo.

##### `cuda` — CUDA execution provider

Backward-compatible shortcut forms keep working:

- `"cuda": true` — enable CUDA on `device_id: 0`
- `"cuda": false` — disable CUDA (default)
- `"cuda": <integer>` — enable CUDA on the given device ID
- `"cuda": { ... }` — object form with CUDA EP V2 keys, including:
    - `device_id` (int)
    - `gpu_mem_limit` (int, bytes)
    - `arena_extend_strategy` (`"kNextPowerOfTwo"` | `"kSameAsRequested"`)
    - `cudnn_conv_algo_search` (`"EXHAUSTIVE"` | `"HEURISTIC"` | `"DEFAULT"`)
    - `cudnn_conv_use_max_workspace` (bool)
    - `do_copy_in_default_stream` (bool)
    - `enable_cuda_graph` (bool)

```json
{
  "cuda": {
    "device_id": 0,
    "gpu_mem_limit": 2147483648,
    "arena_extend_strategy": "kSameAsRequested"
  }
}
```

##### `extensions` — onnxruntime-extensions custom ops

Load custom-op libraries from [onnxruntime-extensions](https://github.com/microsoft/onnxruntime-extensions). The value is an array of absolute paths; libraries are registered in array order.

```json
{
  "extensions": ["/usr/local/lib/libortextensions.so"]
}
```

The legacy single-path form `"ortextensions_path": "<path>"` is still accepted and is normalized into the `extensions` array on input and on echo.

##### `session_options` — ORT `SessionOptions`

Configure the underlying ORT [`SessionOptions`](https://onnxruntime.ai/docs/api/c/struct_ort_api.html). Supported keys:

- `intra_op_num_threads` (int)
- `inter_op_num_threads` (int)
- `execution_mode` (`"ORT_SEQUENTIAL"` | `"ORT_PARALLEL"`)
- `graph_optimization_level` (`"ORT_DISABLE_ALL"` | `"ORT_ENABLE_BASIC"` | `"ORT_ENABLE_EXTENDED"` | `"ORT_ENABLE_ALL"`)
- `enable_cpu_mem_arena` (bool)
- `enable_mem_pattern` (bool)
- `log_severity_level` (int)
- `logid` (string)
- `enable_profiling` (bool, or string prefix)
- `optimized_model_filepath` (string)
- `free_dimension_overrides` (object: `{name: int}`)
- `config_entries` (object: `{key: string-value}`)

```json
{
  "session_options": {
    "intra_op_num_threads": 4,
    "graph_optimization_level": "ORT_ENABLE_ALL",
    "enable_mem_pattern": true,
    "config_entries": { "session.use_env_allocators": "1" }
  }
}
```

##### Combined example

```sql
SELECT pg_onnx_import_model(
        'sample_model',
        'v20230101',
        PG_READ_BINARY_FILE('/your_model_path/model.onnx')::bytea,
        '{
          "cuda": { "device_id": 0, "gpu_mem_limit": 2147483648 },
          "extensions": ["/usr/local/lib/libortextensions.so"],
          "session_options": {
            "intra_op_num_threads": 4,
            "graph_optimization_level": "ORT_ENABLE_ALL"
          }
        }'::jsonb,
        'sample model'
    );
```

----

### pg_onnx_drop_model(TEXT, TEXT)

- Drop imported model.
- Parameters
    - `name(TEXT)`: Model name
    - `version(TEXT)`: Model version
- Returns
    - boolean: `true` if the model is successfully dropped, `false` otherwise.

```sql
SELECT pg_onnx_drop_model('sample_model', 'v20230101');
```

```
 pg_onnx_drop_model 
--------------------
 t
```

----

### pg_onnx_list_model()

- List all imported models.
- Returns list of models(SETOF RECORD)
    - `name(TEXT)`: Model name
    - `version(TEXT)`: Model version
    - `option(JSONB)`: Model options
    - `inputs(JSONB)`: Model input names with type and shape
    - `outputs(JSONB)`: Model output names with type and shape
    - `description(TEXT)`: Model description
    - `created_at(TIMESTAMPTZ)`: Model import time

```sql
SELECT *
FROM pg_onnx_list_model();
```

```
     name     | version |       option       |                               inputs                               |           outputs           | description    |          created_at
--------------+---------+--------------------+--------------------------------------------------------------------+-----------------------------+---------------
 sample_model | v1      | {"cuda": true}     | {"x": "float32[-1,1]", "y": "float32[-1,1]", "z": "float32[-1,1]"} | {"output": "float32[-1,1]"} | sample model1  | 2023-09-16 03:38:55.088838
 sample_model | v2      | {}                 | {"input_ids": "int64[-1,512]", "onnx::Equal_1": "int64[-1,512]"}   | {"output": "int64[-1]"}     | sample model2  | 2023-09-16 03:38:55.088838
(2 rows)
```

----

### pg_onnx_inspect_model_bin(BYTEA, JSONB)

- Get the type and shape information of the inputs and outputs of the ONNX file.
- Returns
    - `inputs(JSONB)`: Input type and shape
    - `outputs(JSONB)`: Output type and shape
- The second argument `option(JSONB)` is optional and accepts the same keys as [Model Options](#model-options). The most common ones for inspection are `extensions` (load `onnxruntime-extensions` custom-op libraries before parsing the model) and `session_options.config_entries`.

```sql
-- inspect a model that uses custom ops from onnxruntime-extensions
SELECT *
FROM pg_onnx_inspect_model_bin(
        PG_READ_BINARY_FILE('/your_model_path/model.onnx')::bytea,
        '{"extensions": ["/usr/local/lib/libortextensions.so"]}'::jsonb
    );
```

```sql
SELECT *
FROM pg_onnx_inspect_model_bin(
        PG_READ_BINARY_FILE('/your_model_path/model.onnx')::bytea
    );
```

```
                               inputs                               |           outputs           
--------------------------------------------------------------------+-----------------------------
 {"x": "float32[-1,1]", "y": "float32[-1,1]", "z": "float32[-1,1]"} | {"output": "float32[-1,1]"}
(1 row)
```

----

## ONNX Session Functions

### pg_onnx_create_session(TEXT, TEXT)

- Create a new ONNX session.
- If the model ONNX file is too huge, you might get an error. In this case, change the value of the temp_file_limit
  setting in PostgreSQL to -1 (no limit).
- Parameters
    - `name(TEXT)`: Model name
    - `version(TEXT)`: Model version
- Returns session information(RECORD)
    - `name(TEXT)`: Model name
    - `version(TEXT)`: Model version
    - `created_at(BIGINT)`: Session creation time
    - `last_executed_at(BIGINT)`: Last execution time
    - `execution_count(BIGINT)`: Number of executions
    - `inputs(JSONB)`: Model input names with type and shape
    - `outputs(JSONB)`: Model output names with type and shape
    - `option(JSONB)`: Model options

```sql
SELECT *
FROM pg_onnx_create_session('sample_model', 'v20230101')
```

```
    model     |  version  | created_at | last_executed_at | execution_count |                               inputs                               |           outputs           |     option      
--------------+-----------+------------+------------------+-----------------+--------------------------------------------------------------------+-----------------------------+-----------------
 sample_model | v20230101 | 1694803952 |                0 |               0 | {"x": "float32[-1,1]", "y": "float32[-1,1]", "z": "float32[-1,1]"} | {"output": "float32[-1,1]"} | {"cuda": false}
(1 row)
```

----

### pg_onnx_describe_session(TEXT, TEXT)

- Gets information about an ONNX session.
- Parameters
    - `name(TEXT)`: Model name
    - `version(TEXT)`: Model version
- Returns session information(RECORD)
    - `name(TEXT)`: Model name
    - `version(TEXT)`: Model version
    - `created_at(BIGINT)`: Session creation time
    - `last_executed_at(BIGINT)`: Last execution time
    - `execution_count(BIGINT)`: Number of executions
    - `inputs(JSONB)`: Model input names with type and shape
    - `outputs(JSONB)`: Model output names with type and shape
    - `option(JSONB)`: Model options

```sql
SELECT *
FROM pg_onnx_describe_session('sample_model', 'v20230101')
```

```
    model     |  version  | created_at | last_executed_at | execution_count |                               inputs                               |           outputs           |     option      
--------------+-----------+------------+------------------+-----------------+--------------------------------------------------------------------+-----------------------------+-----------------
 sample_model | v20230101 | 1694803952 |                0 |               0 | {"x": "float32[-1,1]", "y": "float32[-1,1]", "z": "float32[-1,1]"} | {"output": "float32[-1,1]"} | {"cuda": false}
(1 row)
```

----

### pg_onnx_execute_session(TEXT, TEXT, JSONB)

- Execute the ONNX session.
- ${\color{red}Create an ONNX session if it doesn't exist.}$
- Parameters
    - `name(TEXT)`: Model name
    - `version(TEXT)`: Model version
    - `inputs(JSONB)`: Input data
- Returns
    - `outputs(JSONB)`: Output data

```sql
SELECT pg_onnx_execute_session('sample_model', 'v20230101', '{
  "x": [[1], [2], [3]],
  "y": [[3], [4], [5]],
  "z": [[5], [6], [7]]
}');
```

```
                                pg_onnx_execute                                 
--------------------------------------------------------------------------------
 {"output": [[0.7488641738891602], [0.8607008457183838], [0.9725375175476074]]}
(1 row)
```

----

### pg_onnx_destroy_session(TEXT, TEXT, JSONB)

- Destroy the ONNX session. Clean up unused ONNX sessions.
- Parameters
    - `name(TEXT)`: Model name
    - `version(TEXT)`: Model version
- Returns
    - boolean: `true` if the model is successfully destroyed, `false` otherwise.

```sql
SELECT pg_onnx_destroy_session('sample_model', 'v20230101');
```

```
 pg_onnx_destroy_session 
-------------------------
 t
(1 row)
```

----

### pg_onnx_list_session()

- List all active ONNX sessions.
- Returns list of sessions(SETOF RECORD)
    - `name(TEXT)`: Model name
    - `version(TEXT)`: Model version
    - `created_at(BIGINT)`: Session creation time
    - `last_executed_at(BIGINT)`: Last execution time
    - `execution_count(BIGINT)`: Number of executions
    - `inputs(JSONB)`: Model input names with type and shape
    - `outputs(JSONB)`: Model output names with type and shape
    - `option(JSONB)`: Model options

```sql
SELECT *
FROM pg_onnx_list_model();
```

```
    model     |  version  | created_at | last_executed_at | execution_count |                               inputs                               |           outputs           |     option      
--------------+-----------+------------+------------------+-----------------+--------------------------------------------------------------------+-----------------------------+-----------------
 sample_model | v20230101 | 1694803952 |                0 |               0 | {"x": "float32[-1,1]", "y": "float32[-1,1]", "z": "float32[-1,1]"} | {"output": "float32[-1,1]"} | {"cuda": false}
(1 row)
```

