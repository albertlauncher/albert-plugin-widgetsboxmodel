// Copyright (c) 2024-2025 Manuel Schneider

#pragma once
#include <QBrush>
#include <expected>
#include <filesystem>
#include <format>
#include <map>
#include <vector>
using namespace Qt::StringLiterals; // not that nice, style.cpp is the only includer

template<std::floating_point T>
static std::expected<T, QString> parse(const QString &v)
{
    bool ok = false;
    if (const auto f = v.toDouble(&ok); !ok)
        return std::unexpected(u"Invalid floating-point number: %1"_s.arg(v));
    else if (f < std::numeric_limits<T>::lowest() || f > std::numeric_limits<T>::max())
        return std::unexpected(u"Floating-point number out of range: %1"_s.arg(f));
    else
        return static_cast<T>(f);
}

template<std::integral T>
static std::expected<T, QString> parse(const QString& v)
{
    bool ok = false;
    if (const auto i = v.toLongLong(&ok); !ok)
        return std::unexpected(u"Invalid integer: %1"_s.arg(v));
    else if (i < std::numeric_limits<T>::lowest() || i > std::numeric_limits<T>::max())
        return std::unexpected(u"Integer out of range [%2  %3]: %1 "_s.arg(v));
    else
        return static_cast<T>(i);
}

class StyleFileParser
{
public:
    const std::filesystem::path &path;
    const std::map<QString, QString> raw_entries;
    const std::vector<StyleFileParser> bases;

    StyleFileParser(const std::filesystem::path &);


    // Lookup

    template<typename T>
    T getMandatory(const QString &k) const
    {
        if (auto it = raw_entries.find(k); it == raw_entries.end())
            throw std::runtime_error(std::format("Key not found: %1", k.toStdString()));
        else if (auto exp = parseResolved<T>(it->second))
            return *exp;
        else
            throw std::runtime_error(u"%1 > %2"_s.arg(k, exp.error()).toStdString());
    }

    template<typename T>
    std::optional<T> getOptional(const QString &k) const
    {
        if (auto it = raw_entries.find(k); it == raw_entries.end())
            return std::nullopt;
        else if (auto exp = parseResolved<T>(it->second))
            return *exp;
        else
            throw std::runtime_error(u"%1 > %2"_s.arg(k, exp.error()).toStdString());
    }

    template<typename T>
    void getOptional(const QString &k, T *out) const
    {
        if (auto opt = getOptional<T>(k))
            *out = *opt;
    }

    template<typename T>
    std::expected<T, QString> parseResolved(const QString &v) const
    {
        if (v.startsWith(u'$'))
        {
            const auto key = v.mid(1);
            if (auto it = raw_entries.find(key); it == raw_entries.end())
                return std::unexpected(u"Key not found: %1"_s.arg(key));
            else
                return parseResolved<T>(it->second)
                    .transform_error([&](auto err) { return u"%1 > %2"_s.arg(key, err); });
        }
        else
            return parse<T>(v);
    }


    // Parsing

    struct Fn {
        QString name;
        struct Args {
            QStringList pos;
            std::map<QString, QString> kw;
        } args;
    };

    std::optional<std::expected<Fn, QString>> tryParseFunction(const QString &) const;
    std::expected<QBrush, QString> fnLinearGradient(const Fn::Args &) const;
    std::expected<QBrush, QString> fnImage(const Fn::Args &) const;
    std::expected<QColor, QString> fnChange(const Fn::Args &) const;
    std::expected<QColor, QString> fnAdjust(const Fn::Args &) const;
    std::expected<QColor, QString> fnScale(const Fn::Args &) const;
    std::expected<QColor, QString> fnMultiply(const Fn::Args &) const;

    template<typename T>
    std::expected<T, QString> parse(const QString &) const;

    template<std::floating_point T>
    std::expected<T, QString> parse(const QString &v) const
    { return ::parse<T>(v); }

    template<std::integral T>
    std::expected<T, QString> parse(const QString &v) const
    { return ::parse<T>(v); }

};


template<>
std::expected<QBrush, QString> StyleFileParser::parse(const QString &v) const;

template<>
std::expected<QColor, QString> StyleFileParser::parse(const QString &v) const;


