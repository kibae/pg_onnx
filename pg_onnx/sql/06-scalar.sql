-- ReduceSum fixture reported upstream; embedded to avoid reliance on fixture caches.
DO $$
BEGIN
    PERFORM pg_onnx_import_model('scalar_model', '1', decode('080912076861726e6573733a6f0a280a01780a0461786573120179220952656475636553756d2a0f0a086b65657064696d731800a001021201672a1c080210074204617865734a10000000000000000001000000000000005a150a017812100a0e0801120a0a031201610a03120162620b0a017912060a040801120042040a00100d', 'hex'), '{}', 'scalar regression');
    PERFORM pg_onnx_create_session('scalar_model', '1');
    IF pg_onnx_execute_session('scalar_model', '1', '{"x":[[1,2],[3,4]]}')::jsonb IS DISTINCT FROM '{"y":[10.0]}'::jsonb THEN
        RAISE EXCEPTION 'Unexpected scalar inference result';
    END IF;
    PERFORM pg_onnx_destroy_session('scalar_model', '1');
    PERFORM pg_onnx_drop_model('scalar_model', '1');
END
$$;
