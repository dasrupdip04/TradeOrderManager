# TradeFlow API Reference

## Health

### GET /health

Response:

```json
{"status": "ok"}
```

## Orders

### POST /api/orders

Creates a BUY or SELL order.

Request example:

```json
{
  "user_id": 1,
  "symbol": "RELIANCE",
  "side": "BUY",
  "order_type": "LIMIT",
  "quantity": 100,
  "price": 145050,
  "idempotency_key": "demo-order-001"
}
```

Successful response (
`201`):

```json
{
  "order_id": 1,
  "user_id": 1,
  "symbol": "RELIANCE",
  "side": "BUY",
  "order_type": "LIMIT",
  "quantity": 100,
  "price": 145050,
  "status": "FILLED",
  "idempotency_key": "demo-order-001",
  "executed_quantity": 100,
  "execution_price": 145050
}
```

### GET /api/orders

Returns all orders.

### GET /api/orders/{id}

Returns one order by id.

### DELETE /api/orders/{id}

Cancels an order.

## Positions

### GET /api/positions

Returns all positions.

### GET /api/positions/{symbol}

Returns the position for one symbol.

## P&L

### GET /api/pnl?user_id=1&symbol=RELIANCE

Returns realized and unrealized P&L for a given user/symbol.

## Risk

### GET /api/risk?user_id=1

Returns the current risk limit configuration for that user.

### PUT /api/risk/limits

Updates risk limits for a user.

Example payload:

```json
{
  "user_id": 1,
  "max_order_quantity": 1000,
  "max_position_quantity": 2000,
  "max_notional": 10000000
}
```

## Error format

```json
{
  "error": "RISK_LIMIT_EXCEEDED",
  "message": "Order would exceed maximum position limit"
}
```
