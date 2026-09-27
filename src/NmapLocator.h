#pragma once

#include <QString>

#include <optional>

// Returns the path of the nmap executable, or std::nullopt if none was found.
//
// Search order: overridePath (the path the user chose) -> PATH -> known
// per-OS install locations. If overridePath is set but not a usable
// executable, the result is std::nullopt: silently using a different nmap
// would hide the user's broken setting.
std::optional<QString> findNmap(const QString &overridePath);
