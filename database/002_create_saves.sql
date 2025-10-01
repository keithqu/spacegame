-- saves table
CREATE TABLE IF NOT EXISTS public.saves (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID NOT NULL REFERENCES public.users(id) ON DELETE CASCADE,
    save_slot INTEGER NOT NULL CHECK (save_slot >= 1 AND save_slot <= 10),
    save_data JSONB NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    UNIQUE(user_id, save_slot)
);

-- trigger to update updated_at
DROP TRIGGER IF EXISTS saves_set_updated_at ON public.saves;
CREATE TRIGGER saves_set_updated_at
BEFORE UPDATE ON public.saves
FOR EACH ROW
EXECUTE PROCEDURE set_updated_at();


