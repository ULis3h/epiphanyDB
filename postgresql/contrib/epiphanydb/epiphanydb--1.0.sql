-- EpiphanyDB Extension Installation Script
-- Version 1.0

-- Set search path to include epiphanydb schema
SET search_path = epiphanydb, public;
-- Create storage engine type enum
CREATE TYPE storage_engine_type AS ENUM (
    'heap',
    'columnar', 
    'vector',
    'timeseries',
    'graph'
);

-- Function to get EpiphanyDB version
CREATE OR REPLACE FUNCTION epiphanydb_version()
RETURNS text
AS 'MODULE_PATHNAME', 'epiphanydb_version'
LANGUAGE C IMMUTABLE STRICT;

-- Function to check if EpiphanyDB is enabled
CREATE OR REPLACE FUNCTION epiphanydb_is_enabled()
RETURNS boolean
AS 'MODULE_PATHNAME', 'epiphanydb_is_enabled'
LANGUAGE C IMMUTABLE STRICT;

-- Function to get enabled storage engines
CREATE OR REPLACE FUNCTION epiphanydb_storage_engines_enabled()
RETURNS text
AS 'MODULE_PATHNAME', 'epiphanydb_storage_engines_enabled'
LANGUAGE C IMMUTABLE STRICT;

-- Function to create table with specific storage engine
CREATE OR REPLACE FUNCTION epiphanydb_create_table(
    table_name text,
    storage_type storage_engine_type,
    schema_definition text
)
RETURNS boolean
AS 'MODULE_PATHNAME', 'epiphanydb_create_table_sql'
LANGUAGE C STRICT;

-- Function to get table storage engine
CREATE OR REPLACE FUNCTION epiphanydb_get_table_storage_engine(
    table_name text
)
RETURNS storage_engine_type
AS 'MODULE_PATHNAME', 'epiphanydb_get_table_storage_engine'
LANGUAGE C STRICT;

-- Function to get table statistics
CREATE OR REPLACE FUNCTION epiphanydb_get_table_stats(
    table_name text,
    OUT num_rows bigint,
    OUT table_size bigint
)
AS 'MODULE_PATHNAME', 'epiphanydb_get_table_stats_sql'
LANGUAGE C STRICT;

-- Function to vacuum table
CREATE OR REPLACE FUNCTION epiphanydb_vacuum_table(
    table_name text
)
RETURNS boolean
AS 'MODULE_PATHNAME', 'epiphanydb_vacuum_table_sql'
LANGUAGE C STRICT;

-- Function to analyze table
CREATE OR REPLACE FUNCTION epiphanydb_analyze_table(
    table_name text
)
RETURNS boolean
AS 'MODULE_PATHNAME', 'epiphanydb_analyze_table_sql'
LANGUAGE C STRICT;

-- Create view for storage engine information
CREATE OR REPLACE VIEW storage_engines AS
SELECT 
    unnest(ARRAY['heap', 'columnar', 'vector', 'timeseries', 'graph']) AS engine_name,
    true AS available;

-- Create view for EpiphanyDB configuration
CREATE OR REPLACE VIEW epiphanydb_config AS
SELECT 
    'version' AS setting,
    epiphanydb_version() AS value
UNION ALL
SELECT 
    'enabled' AS setting,
    epiphanydb_is_enabled()::text AS value
UNION ALL
SELECT 
    'storage_engines' AS setting,
    epiphanydb_storage_engines_enabled() AS value;

-- Grant permissions
GRANT USAGE ON SCHEMA epiphanydb TO PUBLIC;
GRANT SELECT ON ALL TABLES IN SCHEMA epiphanydb TO PUBLIC;
GRANT EXECUTE ON ALL FUNCTIONS IN SCHEMA epiphanydb TO PUBLIC;

-- Create example tables for each storage engine type
COMMENT ON SCHEMA epiphanydb IS 'EpiphanyDB multi-modal storage engine schema';
COMMENT ON TYPE storage_engine_type IS 'Available storage engine types in EpiphanyDB';
COMMENT ON FUNCTION epiphanydb_version() IS 'Returns the version of EpiphanyDB extension';
COMMENT ON FUNCTION epiphanydb_is_enabled() IS 'Returns true if EpiphanyDB is enabled';
COMMENT ON FUNCTION epiphanydb_storage_engines_enabled() IS 'Returns comma-separated list of enabled storage engines';
COMMENT ON FUNCTION epiphanydb_create_table(text, storage_engine_type, text) IS 'Creates a table with specified storage engine';
COMMENT ON VIEW storage_engines IS 'Information about available storage engines';
COMMENT ON VIEW epiphanydb_config IS 'EpiphanyDB configuration information';