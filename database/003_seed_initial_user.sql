-- seed initial user 'keith'
INSERT INTO public.users (username, email, membership)
VALUES ('keith', 'kqu123@gmail.com', NULL)
ON CONFLICT (username) DO UPDATE SET email = EXCLUDED.email;


