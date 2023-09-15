SELECT pg_onnx_create_model(
               'sample_model',
               '0',
               ''::BYTEA
           );

SELECT name,
       version,
       model,
       option,
       description
FROM ext_pg_onnx.model
ORDER BY 1, 2;

SELECT pg_onnx_create_model(
               'sample_model',
               '0',
               ''::BYTEA
           );

SELECT pg_onnx_create_model(
               'sample_model',
               '1',
               PG_READ_BINARY_FILE('../../../onnxruntime-server/test/fixture/sample/1/model.onnx')::bytea,
               '{}',
               'sample model'
           );

SELECT name,
       version,
       model,
       option,
       description
FROM ext_pg_onnx.model
ORDER BY 1, 2;

SELECT pg_onnx_create_model(
               'sample_model',
               '2',
               PG_READ_BINARY_FILE('../../../onnxruntime-server/test/fixture/sample/2/model.onnx')::bytea,
               '{}',
               'sample model'
           );
