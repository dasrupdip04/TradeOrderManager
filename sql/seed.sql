INSERT INTO users (username, email) VALUES
    ('alice', 'alice@tradeflow.local'),
    ('bob', 'bob@tradeflow.local')
ON CONFLICT (username) DO NOTHING;

INSERT INTO risk_limits (user_id, max_order_quantity, max_position_quantity, max_notional)
SELECT id, 1000, 1000, 1000000 FROM users WHERE username = 'alice'
ON CONFLICT (user_id) DO UPDATE SET
    max_order_quantity = EXCLUDED.max_order_quantity,
    max_position_quantity = EXCLUDED.max_position_quantity,
    max_notional = EXCLUDED.max_notional;

INSERT INTO risk_limits (user_id, max_order_quantity, max_position_quantity, max_notional)
SELECT id, 500, 500, 500000 FROM users WHERE username = 'bob'
ON CONFLICT (user_id) DO UPDATE SET
    max_order_quantity = EXCLUDED.max_order_quantity,
    max_position_quantity = EXCLUDED.max_position_quantity,
    max_notional = EXCLUDED.max_notional;
