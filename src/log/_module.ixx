/**
 * @file _module.ixx
 * @brief Module exports for logging.
 */

/**
 * @brief Logging system for debugging and diagnostics.
 *
 * @details This module provides logging functionality including logger instances,
 * log management, and configurable output sinks for flexible log destinations.
 */
export module helios.core.log;

export import helios.core.log.LogSink;
export import helios.core.log.ConsoleSink;
export import helios.core.log.Logger;
export import helios.core.log.LogManager;