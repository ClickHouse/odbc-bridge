#include "IdentifierQuoteHandler.h"
#if USE_POCO_SQLODBC || USE_POCO_DATAODBC

#if USE_POCO_SQLODBC
#include <Poco/SQL/ODBC/ODBCException.h>
#include <Poco/SQL/ODBC/SessionImpl.h>
#include <Poco/SQL/ODBC/Utility.h>
#define POCO_SQL_ODBC_CLASS Poco::SQL::ODBC
#endif
#if USE_POCO_DATAODBC
#include <Poco/Data/ODBC/ODBCException.h>
#include <Poco/Data/ODBC/SessionImpl.h>
#include <Poco/Data/ODBC/Utility.h>
#define POCO_SQL_ODBC_CLASS Poco::Data::ODBC
#endif

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTMLForm.h>
#include <DataTypes/DataTypeFactory.h>
#include <IO/WriteBufferFromHTTPServerResponse.h>
#include <IO/WriteHelpers.h>
#include <Parsers/ParserQueryWithOutput.h>
#include <Parsers/parseQuery.h>
#include <common/logger_useful.h>
#include <ext/scope_guard.h>
#include "validateODBCConnectionString.h"

namespace DB
{

void IdentifierQuoteHandler::handleRequest(Poco::Net::HTTPServerRequest & request, Poco::Net::HTTPServerResponse & response)
{
    Poco::Net::HTMLForm params(request, request.stream());
    LOG_TRACE(log, "Request URI: " + request.getURI());

    auto process_error = [&response, this](const std::string & message)
    {
        response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        if (!response.sent())
            response.send() << message << std::endl;
        LOG_WARNING(log, message);
    };

     if (!params.has("connection_string"))
    {
        process_error("No 'connection_string' in request URL");
        return;
    }

    try
    {
        std::string connection_string = params.get("connection_string");
        POCO_SQL_ODBC_CLASS::SessionImpl session(validateODBCConnectionString(connection_string), DBMS_DEFAULT_CONNECT_TIMEOUT_SEC);
        SQLHDBC hdbc = session.dbc().handle();

        std::string identifier;

        SQLSMALLINT t;
        SQLRETURN r = Poco::Data::ODBC::SQLGetInfo(hdbc, SQL_IDENTIFIER_QUOTE_CHAR, NULL, 0, &t);
        if (!POCO_SQL_ODBC_CLASS::Utility::isError(r) && t > 0)
        {
            identifier.resize(static_cast<std::size_t>(t) + 2);
            r = Poco::Data::ODBC::SQLGetInfo(hdbc, SQL_IDENTIFIER_QUOTE_CHAR, &identifier[0], SQLSMALLINT((identifier.length() - 1) * sizeof(identifier[0])), &t);
            LOG_TRACE(log, "GOT IDENTIFIER: '" + identifier + "'");
        }

        LOG_TRACE(log, "FINAL IDENTIFIER: '" + identifier + "'");

        WriteBufferFromHTTPServerResponse out(request, response, keep_alive_timeout);
        writeStringBinary(identifier, out);
    }
    catch (...)
    {
        process_error("Error getting identifier quote style from ODBC '" + getCurrentExceptionMessage(false) + "'");
        tryLogCurrentException(log);
    }
}
}
#endif
