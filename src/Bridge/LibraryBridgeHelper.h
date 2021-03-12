#pragma once

#include <Interpreters/Context.h>
#include <IO/ReadWriteBufferFromHTTP.h>
#include <Poco/Logger.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/URI.h>
#include <Bridge/IBridgeHelper.h>


namespace DB
{

class LibraryBridgeHelper : public IBridgeHelper
{

public:
    static constexpr inline size_t DEFAULT_PORT = 9012;

    LibraryBridgeHelper(const Context & context_, const std::string & dictionary_id_);

    bool initLibrary(const std::string & library_path, const std::string library_settings);

    bool cloneLibrary(const std::string & other_dictionary_id);

    bool removeLibrary();

    bool isModified();

    bool supportsSelectiveLoad();

    BlockInputStreamPtr loadAll(const std::string attributes_string, const Block & sample_block);

    BlockInputStreamPtr loadIds(const std::string attributes_string, const std::string ids_string, const Block & sample_block);

    BlockInputStreamPtr loadKeys(const Block & key_columns, const Block & sample_block);

    BlockInputStreamPtr loadBase(const Poco::URI & uri, const Block & sample_block, ReadWriteBufferFromHTTP::OutStreamCallback out_stream_callback = {});

    bool executeRequest(const Poco::URI & uri);


protected:
    void startBridge(std::unique_ptr<ShellCommand> cmd) const override;

    const String serviceAlias() const override { return "clickhouse-library-bridge"; }

    const String serviceFileName() const override { return serviceAlias(); }

    size_t getDefaultPort() const override { return DEFAULT_PORT; }

    bool startBridgeManually() const override { return false; }

    const String configPrefix() const override { return "library_bridge"; }

    const Context & getContext() const override { return context; }

    const Poco::Util::AbstractConfiguration & getConfig() const override { return config; }

    Poco::Logger * getLog() const override { return log; }

    const Poco::Timespan & getHTTPTimeout() const override { return http_timeout; }

    Poco::URI createBaseURI() const override;

private:
    static constexpr inline auto LIB_NEW_METHOD = "libNew";
    static constexpr inline auto LIB_CLONE_METHOD = "libClone";
    static constexpr inline auto LIB_DELETE_METHOD = "libDelete";
    static constexpr inline auto LOAD_ALL_METHOD = "loadAll";
    static constexpr inline auto LOAD_IDS_METHOD = "loadIds";
    static constexpr inline auto LOAD_KEYS_METHOD = "loadKeys";
    static constexpr inline auto IS_MODIFIED_METHOD = "isModified";
    static constexpr inline auto SUPPORTS_SELECTIVE_LOAD_METHOD = "supportsSelectiveLoad";

    Poco::URI getDictionaryURI() const;

    Poco::Logger * log;
    const Context & context;
    const Poco::Util::AbstractConfiguration & config;
    const Poco::Timespan http_timeout;

    const std::string dictionary_id;
    std::string bridge_host;
    size_t bridge_port;
};
}
