# ClickHouse ODBC Bridge

This repository contains a plugin for ClickHouse that lets it run federated queries on other databases using their ODBC interface. This plugin was introduced in ClickHouse version 18.12 in Aug 2018.

Don't be confused with another repository, [clickhouse-odbc](https://github.com/ClickHouse/clickhouse-odbc/), which provides an official ODBC driver for ClickHouse.

# About

[ClickHouse](https://github.com/ClickHouse/ClickHouse) database integrates with a variety of external data sources, including external DBMS, for federated queries. Some of these databases, such as MySQL, PostgreSQL, MongoDB, and Redis, are supported through native integration, and they can also utilize ODBC and JDBC drivers to integrate with any other compatible database.

[ODBC](https://en.wikipedia.org/wiki/Open_Database_Connectivity) provides a standard interface for such plugins as C libraries. It became popular for using ClickHouse to query data from Oracle and MS SQL Server.  

# Motivation

ODBC works using dynamically loaded third-party shared libraries (.dll, .so). It is represented by a "driver manager" which reads the configuration of data sources, and loads drivers. Each driver is represented by a shared library, which is loaded dynamically. The application uses a driver manager to load drivers. Then it calls the methods of the driver to run queries and read results.

If an application loads drivers directly, this will provide a bunch of security and stability problems:
- third-party libraries cannot be tested with sanitizers;
- third-party libraries may expose unneeded symbols that conflict with the main program symbols;
- third-party libraries may break ABI subtly;
- the main program has to be linked dynamically, which is also insecure;
- the driver manager may be asked to load arbitrary shared libraries from the filesystem, which is insecure;
- loading shared libraries in runtime is a complicated process with many options and a lot of fragility;
- third-party code in the program's address space implies memory-safety issues;
- the set of possible drivers is unknown in advance, and if a problem arises from the usage of a particular driver, it makes debugging hardly possible;
- drivers cannot be trusted, and besides vulnerabilities, they can do excessive resource consumption, exhaust memory, make busy loops, create threads, print logs uncontrollably, etc.

It's obvious that no reasonable product should load dynamic libraries at runtime. At the same time, we want to use ODBC drivers. To resolve this paradox, we split the mechanics of integration with ODBC into a separate program, `clickhouse-odbc-bridge`.

`clickhouse-odbc-bridge` is a small isolated program, that uses ODBC drivers and provides an HTTP interface to interact with it. The queries have to be passed using an HTTP POST or GET request, and the results are serialized using one of the supported formats, such as TSV. This means that possible bugs inside third-party ODBC drivers don't affect our precious, solid, self-contained, and self-confident `clickhouse-server`.

But it still requires extra care to avoid potential vulnerabilities related to ODBC:
- we prevent using ODBC connection strings to specify paths to drivers at run time;
- the operator should install only trusted ODBC drivers and never allow external users to control the set of available drivers;
- as an additional security measure, it makes sense to run the ODBC bridge in a separate machine, or in a separate container.

# Installation

Find the `clickhouse-odbc-bridge` deb, rpm, or tgz package at https://packages.clickhouse.com/deb/pool/main/c/clickhouse/ and install it.

The version of `clickhouse-odbc-bridge` does not have to correspond to `clickhouse-server`. All versions are compatible with each other. You can use the latest ODBC bridge with any ClickHouse version.

# Documentation

Usage of the `odbc` integration for federated queries: https://clickhouse.com/docs/en/sql-reference/table-functions/odbc

Command line usage of the ODBC bridge: https://clickhouse.com/docs/en/operations/utilities/odbc-bridge

# Release Schedule

ClickHouse ODBC Bridge is a simple and stable program. We don't plan to release patches and updates for years. Do not ask for updates. Install it and mind your business.

# How to Build

Clone the main ClickHouse repository, https://github.com/ClickHouse/ClickHouse, switch to commit `54097d0e19c36977e3361b17e8dc897cd9e5aebc`, optionally copy all the files from this repository there, and build it as usual: https://clickhouse.com/docs/en/development/build 

# See Also

This repository also contains ClickHouse Library Bridge, which allows ClickHouse to work with [CatBoost](https://github.com/catboost/catboost/) and obsolete library dictionaries.
