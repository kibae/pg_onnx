CREATE SCHEMA ext_pg_onnx;

CREATE TABLE ext_pg_onnx.model
(
    name        TEXT                     NOT NULL,
    version     TEXT                     NOT NULL,
    option      JSONB                    NOT NULL DEFAULT '{}'::JSONB,
    inputs      JSONB                    NOT NULL DEFAULT '{}'::JSONB,
    outputs     JSONB                    NOT NULL DEFAULT '{}'::JSONB,
    description TEXT                     NULL,
    created_at  TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT NOW(),
    lo_oid      OID,
    PRIMARY KEY (name, version)
);

CREATE TYPE pg_onnx_inspect AS
(
    inputs  jsonb,
    outputs jsonb
);

CREATE FUNCTION pg_onnx_inspect_model_bin(model bytea)
    RETURNS pg_onnx_inspect
AS
'MODULE_PATHNAME',
'pg_onnx_inspect_model_bin'
    LANGUAGE C STRICT;


CREATE OR REPLACE FUNCTION pg_onnx_list_model()
    RETURNS SETOF ext_pg_onnx.model
AS
$$
SELECT *
FROM ext_pg_onnx.model
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION pg_onnx_import_model(
    name TEXT,
    version TEXT,
    model BYTEA,
    option JSONB DEFAULT '{}'::JSONB,
    description TEXT DEFAULT NULL
) RETURNS BOOLEAN AS
$$
DECLARE
    model_oid OID;
    result    BOOLEAN;
    inspect   pg_onnx_inspect;
BEGIN
    inspect := pg_onnx_inspect_model_bin($3);
    model_oid := lo_from_bytea(0, $3);

    -- RAISE NOTICE 'model_oid: %', model_oid;

    WITH res
        AS (
            INSERT INTO ext_pg_onnx.model
                (name, version, option, description, inputs, outputs, lo_oid)
                VALUES ($1, $2, $4, $5, inspect.inputs, inspect.outputs, model_oid)
                RETURNING model.name, model.version)
    SELECT CASE WHEN COUNT(*) > 0 THEN TRUE ELSE FALSE END
    INTO result
    FROM res;

    RETURN result;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION pg_onnx_drop_model(
    name TEXT,
    version TEXT
) RETURNS BOOLEAN AS
$$
DECLARE
    result BOOLEAN;
BEGIN
    WITH res
        AS (
            DELETE FROM ext_pg_onnx.model
                WHERE model.name = $1 AND model.version = $2
                RETURNING model.name, model.version, model.lo_oid),
        lo_del AS (SELECT lo_unlink(res.lo_oid) FROM res)
    SELECT CASE
               WHEN COUNT(*) > 0 THEN TRUE
               ELSE FALSE END
    INTO result
    FROM res;

    RETURN result;
END;
$$ LANGUAGE plpgsql;



CREATE TYPE pg_onnx_session AS
(
    model            TEXT,
    version          TEXT,
    created_at       BIGINT,
    last_executed_at BIGINT,
    execution_count  BIGINT,
    inputs           jsonb,
    outputs          jsonb,
    option           jsonb
);

CREATE FUNCTION pg_onnx_list_session()
    RETURNS SETOF pg_onnx_session
AS
'MODULE_PATHNAME',
'pg_onnx_internal_list_session'
    LANGUAGE C STRICT;

CREATE FUNCTION pg_onnx_create_session(name TEXT, version TEXT)
    RETURNS SETOF pg_onnx_session
AS
'MODULE_PATHNAME',
'pg_onnx_internal_create_session'
    LANGUAGE C
    STRICT;

CREATE FUNCTION pg_onnx_describe_session(name TEXT, version TEXT)
    RETURNS SETOF pg_onnx_session
AS
'MODULE_PATHNAME',
'pg_onnx_internal_describe_session'
    LANGUAGE C
    STRICT;

CREATE FUNCTION pg_onnx_destroy_session(name TEXT, version TEXT)
    RETURNS BOOLEAN
AS
'MODULE_PATHNAME',
'pg_onnx_internal_destroy_session'
    LANGUAGE C
    STRICT;

CREATE FUNCTION pg_onnx_execute_session(name TEXT, version TEXT, inputs jsonb)
    RETURNS jsonb
AS
'MODULE_PATHNAME',
'pg_onnx_internal_execute_session'
    LANGUAGE C IMMUTABLE
               STRICT;



-- CREATE TABLE ext_pg_onnx.model_statistic
-- (
--     name       TEXT                     NOT NULL,
--     version    TEXT                     NOT NULL,
--
--     execute    BIGINT                   NOT NULL DEFAULT 0,
--     success    BIGINT                   NOT NULL DEFAULT 0,
--     fail       BIGINT                   NOT NULL DEFAULT 0,
--
--     updated_at TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT NOW(),
--     PRIMARY KEY (name, version),
--     FOREIGN KEY (name, version) REFERENCES ext_pg_onnx.model (name, version) ON DELETE CASCADE
-- );

-- When a record is inserted into pg_onnx.model, it is also inserted into pg_onnx.model_statistic
-- CREATE OR REPLACE FUNCTION ext_pg_onnx.model_statistic_insert()
--     RETURNS TRIGGER AS
-- $$
-- BEGIN
--     INSERT INTO ext_pg_onnx.model_statistic (name, version) VALUES (NEW.name, NEW.version);
--     RETURN NEW;
-- END;
-- $$
--     LANGUAGE plpgsql;
--
-- CREATE TRIGGER model_statistic_insert
--     AFTER INSERT
--     ON ext_pg_onnx.model
--     FOR EACH ROW
-- EXECUTE PROCEDURE ext_pg_onnx.model_statistic_insert();

