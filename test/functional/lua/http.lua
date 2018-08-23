local rspamd_http = require "rspamd_http"
local rspamd_logger = require "rspamd_logger"

local function http_symbol(task)

  local url = tostring(task:get_request_header('url'))
  local method = tostring(task:get_request_header('method'))

  task:insert_result('method_' .. method, 1.0)

  local function http_callback(err, code, body)
    if err then
      rspamd_logger.errx('http_callback error: ' .. err)
      task:insert_result('HTTP_ERROR', 1.0, err)
    else
      task:insert_result('HTTP_' .. code, 1.0, body)
    end
  end

  local function http_dns_callback(err, code, body)
    if err then
      rspamd_logger.errx('http_dns_callback error: ' .. err)
      task:insert_result('HTTP_DNS_ERROR', 1.0, err)
    else
      task:insert_result('HTTP_DNS_' .. code, 1.0, body)
    end
  end

  rspamd_http.request({
    url = 'http://127.0.0.1:18080' .. url,
    task = task,
    method = method,
    callback = http_callback,
    timeout = 1,
  })

  --[[ request to this address involved DNS resolver subsystem ]]
  rspamd_http.request({
    url = 'http://site.resolveme:18080' .. url,
    task = task,
    method = method,
    callback = http_dns_callback,
    timeout = 1,
  })

  local err, response = rspamd_http.request({
    url = 'http://127.0.0.1:18080' .. url,
    task = task,
    method = method,
    timeout = 1,
  })

  if not err then
    task:insert_result('HTTP_CORO_' .. response.code, 1.0, response.content)
  else
    task:insert_result('HTTP_CORO_ERROR', 1.0, err)
  end

  err, response = rspamd_http.request({
    url = 'http://site.resolveme:18080' .. url,
    task = task,
    method = method,
    timeout = 1,
  })

  if not err then
    task:insert_result('HTTP_CORO_DNS_' .. response.code, 1.0, response.content)
  else
    task:insert_result('HTTP_CORO_DNS_ERROR', 1.0, err)
  end
end

rspamd_config:register_symbol({
  name = 'SIMPLE_TEST',
  score = 1.0,
  callback = http_symbol,
  no_squeeze = true
})
