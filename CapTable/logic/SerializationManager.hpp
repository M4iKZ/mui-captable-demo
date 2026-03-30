#pragma once

#include <string>

#include "Models.hpp"

namespace mUI::Demo::CapTable
{
    /**
     * @brief JSON serialization / deserialization for CapTableState.
     *
     * Uses nlohmann/json (header-only). Files are plain UTF-8 JSON.
     * No exceptions propagate out of these methods: all errors are
     * reported via the returned bool.
     */
    class SerializationManager
    {
    public:

        SerializationManager() = default;

        /**
         * @brief Serialize state to a JSON string.
         *
         * @param state   State to serialize.
         * @param pretty  When true, output is indented (human-readable).
         * @return        JSON string, or empty on failure.
         */
        std::string toJson(const CapTableState& state, bool pretty = true) const;

        /**
         * @brief Populate a CapTableState from a JSON string.
         *
         * Unknown / missing fields are silently ignored so old exports
         * remain forward-compatible.
         *
         * @param json   UTF-8 JSON string.
         * @param state  Output state (written only on success).
         * @return       true on success.
         */
        bool fromJson(const std::string& json, CapTableState& state) const;

        /**
         * @brief Write state to a UTF-8 file.
         *
         * @param path   Absolute file path.
         * @param state  State to save.
         * @return       true on success.
         */
        bool saveToFile(const std::string& path, const CapTableState& state) const;

        /**
         * @brief Load state from a UTF-8 file.
         *
         * @param path   Absolute file path.
         * @param state  Output state (written only on success).
         * @return       true on success.
         */
        bool loadFromFile(const std::string& path, CapTableState& state) const;

    };

} // namespace mUI::Demo::CapTable