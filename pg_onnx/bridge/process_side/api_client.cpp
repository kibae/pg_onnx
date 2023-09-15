//
// Created by Kibae Shin on 2023/09/13.
//

#include "process_side.hpp"

#include "transport/tcp/tcp_server.hpp"

json api_request(
	extension_state_t *state, int16_t type, const json &request_json, const char *post, size_t post_length
) {
	// elog(LOG, "api_request: %d, %s", type, body.dump().c_str());

	auto json = request_json.dump();
	struct onnxruntime_server::transport::tcp::protocol_header header = {0, 0, 0, 0};
	header.type = htons(type);
	header.json_length = htonll(json.size());
	header.post_length = htonll(post_length);
	header.length = header.json_length + header.post_length;

	boost::asio::io_context io_context;
	boost::asio::ip::tcp::socket socket(io_context);
	// TODO: Use state->config_onnxruntime_server_hostname, state->config_onnxruntime_server_port for remote standalone
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string("127.0.0.1"), state->port);

	boost::system::error_code ec;
	socket.connect(endpoint, ec);
	if (ec)
		throw std::runtime_error(ec.message());

	std::vector<boost::asio::const_buffer> buffers;
	buffers.emplace_back(&header, sizeof(header));
	buffers.emplace_back(json.data(), json.size());
	if (post != nullptr)
		buffers.emplace_back(post, post_length);

	boost::asio::write(socket, buffers, ec);
	if (ec)
		throw std::runtime_error(ec.message());

	struct Orts::transport::tcp::protocol_header res_header;
	std::string res_data;
	while (true) {
		char buffer[1024 * 4];
		std::size_t bytes_read = socket.read_some(boost::asio::buffer(buffer), ec);
		if (ec)
			throw std::runtime_error(ec.message());
		res_data.append(buffer, bytes_read);

		if (res_data.size() > sizeof(struct Orts::transport::tcp::protocol_header)) {
			res_header = *(struct Orts::transport::tcp::protocol_header *)res_data.data();
			res_header.type = ntohs(res_header.type);
			res_header.length = ntohll(res_header.length);

			if (res_data.length() >= res_header.length)
				break;
		}
	}

	socket.close();

	return json::parse(res_data.c_str() + sizeof(res_header));
}
