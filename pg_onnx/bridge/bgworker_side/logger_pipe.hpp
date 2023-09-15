//
// Created by Kibae Shin on 2023/09/14.
//

#ifndef PG_ONNX_LOGGER_PIPE_HPP
#define PG_ONNX_LOGGER_PIPE_HPP

class SinkElog : public AixLog::SinkCout {
  public:
	SinkElog() : SinkCout(AixLog::Filter()) {
	}

	void log(const AixLog::Metadata &metadata, const std::string &message) override {
		switch (metadata.severity) {
		case AixLog::Severity::trace:
			elog(DEBUG1, "%s", message.c_str());
			break;
		case AixLog::Severity::debug:
			elog(DEBUG2, "%s", message.c_str());
			break;
		case AixLog::Severity::info:
			elog(INFO, "%s", message.c_str());
			break;
		case AixLog::Severity::notice:
			elog(NOTICE, "%s", message.c_str());
			break;
		case AixLog::Severity::warning:
			elog(WARNING, "%s", message.c_str());
			break;
		case AixLog::Severity::error:
			elog(ERROR, "%s", message.c_str());
			break;
		case AixLog::Severity::fatal:
			elog(FATAL, "%s", message.c_str());
			break;
		}
	}
};

#endif // PG_ONNX_LOGGER_PIPE_HPP
