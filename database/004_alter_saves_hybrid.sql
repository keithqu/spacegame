-- Hybrid approach: keep JSONB metadata, add optional BYTEA blobs for heavy arrays
ALTER TABLE public.saves
    ADD COLUMN IF NOT EXISTS version INTEGER NOT NULL DEFAULT 1,
    ADD COLUMN IF NOT EXISTS planets_blob BYTEA NULL,
    ADD COLUMN IF NOT EXISTS factions_blob BYTEA NULL;

-- Optional: comment columns to document intended usage
COMMENT ON COLUMN public.saves.version IS 'Application save schema version for migrations';
COMMENT ON COLUMN public.saves.planets_blob IS 'Compressed binary payload for planets array (e.g., zstd/protobuf)';
COMMENT ON COLUMN public.saves.factions_blob IS 'Compressed binary payload for factions array (e.g., zstd/protobuf)';


