SELECT *
FROM pg_onnx_inspect_model_bin(
        PG_READ_BINARY_FILE('../../../onnxruntime-server/test/fixture/sample/1/model.onnx')::bytea
    );


SELECT pg_onnx_create_model(
               'sample_model',
               '1',
               PG_READ_BINARY_FILE('../../../onnxruntime-server/test/fixture/sample/1/model.onnx')::bytea,
               '{}',
               'sample model'
           );

SELECT pg_onnx_create_model(
               'sample_model',
               '1',
               PG_READ_BINARY_FILE('../../../onnxruntime-server/test/fixture/sample/1/model.onnx')::bytea,
               '{}',
               'sample model'
           );

SELECT pg_onnx_create_model(
               'sample_model',
               '2',
               PG_READ_BINARY_FILE('../../../onnxruntime-server/test/fixture/sample/2/model.onnx')::bytea,
               '{}',
               'sample model'
           );

SELECT name, version, option, inputs, outputs, description
FROM ext_pg_onnx.model
ORDER BY 1, 2;

SELECT name, version, option, inputs, outputs, description
FROM pg_onnx_list_model()
ORDER BY 1, 2;
