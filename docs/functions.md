# Functions

- [ONNX Model Functions](#onnx-model-functions)
    - [pg_onnx_import_model(TEXT, TEXT, BYTEA, JSONB, TEXT)](#pg_onnx_import_modeltext-text-bytea-jsonb-text)
    - [pg_onnx_drop_model(TEXT, TEXT)](#pg_onnx_drop_modeltext-text)
    - [pg_onnx_list_model()](#pg_onnx_list_model)
    - [pg_onnx_inspect_model_bin(BYTEA)](#pg_onnx_inspect_model_binbytea)
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
- If you want to enable the use of CUDA, use `{"cuda": true}` or `{"cuda": {"device_id": 0}}` to specify the device ID.
- Parameters
    - `name(TEXT)`: Model name
    - `version(TEXT)`: Model version
    - `model(BYTEA)`: ONNX model binary data
    - `option(JSONB)`: Options
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

### pg_onnx_inspect_model_bin(BYTEA)

- Get the type and shape information of the inputs and outputs of the ONNX file.
- Returns
    - `inputs(JSONB)`: Input type and shape
    - `outputs(JSONB)`: Output type and shape

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

