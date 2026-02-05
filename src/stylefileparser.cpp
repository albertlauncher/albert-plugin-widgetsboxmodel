// Copyright (c) 2024-2026 Manuel Schneider

#include "color.h"
#include "stylefileparser.h"
#include <QApplication>
#include <QDir>
#include <QFont>
#include <QLinearGradient>
#include <QRegularExpression>
#include <QSettings>
#include <QStyle>
#include <albert/logging.h>
#include <albert/systemutil.h>
#include <expected>
#include <filesystem>
#include <flat_map>
#include <map>
#include <optional>
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;


namespace
{

static inline const std::flat_map brush_function_map {
    std::pair(u"linear-gradient"_s, &StyleFileParser::fnLinearGradient),
    std::pair(u"image"_s, &StyleFileParser::fnImage),
};

static inline const std::flat_map color_function_map {
    std::pair(u"change"_s, &StyleFileParser::fnChange),
    std::pair(u"adjust"_s, &StyleFileParser::fnAdjust),
    std::pair(u"scale"_s, &StyleFileParser::fnScale),
    std::pair(u"multiply"_s, &StyleFileParser::fnMultiply)
};

template<typename T>
static expected<T, QString> checkRange(T v,
                                       T min = numeric_limits<T>::min(),
                                       T max = numeric_limits<T>::max())
{
    if (v < min || v > max)
        return unexpected(u"Out of range: %1 ∉ [%2, %3] "_s.arg(v).arg(min).arg(max));
    else
        return v;
}

}

StyleFileParser::StyleFileParser(const filesystem::path &p) :
    path(p),
    raw_entries([&] {
        map<QString, QString> ret;
        QSettings ini(toQString(path), QSettings::IniFormat);
        DEBG << "Load style file:" << toQString(path);
        for (auto &key : ini.allKeys())
        {
            auto value = [&]{
                // Fix Qt treating comma-separated strings as QStringList
                auto var = ini.value(key);
                if (var.typeId() == qMetaTypeId<QStringList>())
                    return var.toStringList().join(u","_s);
                else if (var.typeId() == qMetaTypeId<QString>())
                    return var.toString().trimmed();
                else
                    throw runtime_error(format("Unsupported entry {}", key.toStdString()));
            }();

            key = key.replace(u'/', u'.'); // Prefer dotted notation

            if (const auto &[it, succ] =  ret.emplace(key, value); !succ)
                throw runtime_error(format("Duplicate key{}", key.toStdString()));
        }
        return ret;
    }()),
    bases([&] {
        vector<StyleFileParser> ret;
        map<QString, QString> base_entries;
        if (const auto it = raw_entries.find(u"bases"_s);
            it != raw_entries.end())
            for (const auto &base : it->second.split(u","_s, Qt::SkipEmptyParts))
            {
                if (auto base_path = filesystem::path(base.toStdString());
                    base_path.is_absolute() && filesystem::exists(base_path))
                    ret.emplace_back(StyleFileParser(base_path));
                else if (base_path = path.parent_path() / base.toStdString();
                         filesystem::exists(base_path))
                    ret.emplace_back(StyleFileParser(base_path));
                else
                    throw runtime_error(format("Could not find base style: {}", base.toStdString()));
            }
        return ret;
    }())
{
}

static void addArg(StyleFileParser::Fn::Args &args, const QString &s)
{
    static const auto re = QRegularExpression(R"(^(?:\s*(\w+)\s*:\s*(.+))\s*$)"_L1);
    if (auto match = re.match(s); match.hasMatch())
        args.kw.emplace(match.captured(1), match.captured(2));
    else
        args.pos.emplace_back(s);
}

optional<expected<StyleFileParser::Fn, QString>> StyleFileParser::tryParseFunction(const QString &s) const
{
    auto pos = s.indexOf(u'(');
    if (pos <= 0)
        return nullopt; // no open paren or starts with paren -> not a function

    const QString name = s.left(pos);
    static const QRegularExpression re_name(uR"(^[\w-]+$)"_s);
    if (!re_name.match(name).hasMatch())
        return unexpected(u"Invalid function name: %1"_s.arg(name));

    StyleFileParser::Fn fn{.name=name, .args={}};

    int depth = 0;
    for (auto i = pos; i < s.size(); ++i)
    {
        if (s[i] == u'(')
            ++depth;

        else if (s[i] == u')')
        {
            --depth;
            if (depth == 0)
            {
                // must end exactly here
                if (i != s.size() - 1)
                    return unexpected(u"Unexpected characters after function: %1"_s
                                          .arg(s.mid(i + 1)));

                addArg(fn.args, s.mid(pos + 1, i - pos - 1));
                return fn;
            }
            if (depth < 0)
                return unexpected(u"Unmatched parentheses in function: %1"_s.arg(s));
        }
        else if (s[i] == u',' && depth == 1)
        {
            addArg(fn.args, s.mid(pos + 1, i - pos - 1));
            pos = i;
        }
    }

    return unexpected(u"Unmatched parentheses in function: %1"_s.arg(s));
}

expected<QBrush, QString> StyleFileParser::fnLinearGradient(const Fn::Args &args) const
{
    if (args.pos.size() < 6)
        return unexpected(u"linear-gradient: Expects at least six postional arguments."_s);

    const auto exp_x1 = parse<double>(args.pos[0]);
    if (!exp_x1)
        return unexpected(u"linear-gradient: Invalid value for x1: %1"_s.arg(args.pos[0]));

    const auto exp_y1 = parse<double>(args.pos[1]);
    if (!exp_y1)
        return unexpected(u"linear-gradient: Invalid value for y1: %1"_s.arg(args.pos[1]));

    const auto exp_x2 = parse<double>(args.pos[2]);
    if (!exp_x2)
        return unexpected(u"linear-gradient: Invalid value for x2: %1"_s.arg(args.pos[2]));

    const auto exp_y2 = parse<double>(args.pos[3]);
    if (!exp_y2)
        return unexpected(u"linear-gradient: Invalid value for y2: %1"_s.arg(args.pos[3]));

    QGradientStops stops;

    auto it = args.pos.begin() + 4;
    for (; it != args.pos.end(); ++it)
    {
        const auto stop_args = (*it).split(u" "_s, Qt::SkipEmptyParts);
        if (stop_args.size() != 2)
            return unexpected(u"Gradient stop requires two arguments: %1"_s.arg(*it));

        const auto exp_float = parse<double>(stop_args[0]);
        if (!exp_float)
            return unexpected(u"Invalid stop value: %1 (%2)"_s.arg(stop_args[0], exp_float.error()));

        const auto exp_color = parseResolved<QColor>(stop_args[1]);
        if (!exp_color)
            return unexpected(u"Invalid stop color: %1 "_s.arg(exp_color.error()));

        stops.emplace_back(*exp_float, *exp_color);
    }


    QLinearGradient lg(*exp_x1, *exp_y1, *exp_x2, *exp_y2);
    lg.setStops(stops);
    lg.setCoordinateMode(QGradient::ObjectMode);
    return lg;

    // const auto x1_args = args.kw.equal_range(u"x1"_s);
    // const auto y1_args = args.kw.equal_range(u"y1"_s);
    // const auto x2_args = args.kw.equal_range(u"x2"_s);
    // const auto y2_args = args.kw.equal_range(u"y2"_s);

    // if (distance(x1_args.first, x1_args.second) != 1)
    //     return unexpected(u"Exactly one argument required: x1"_s);
    // if (distance(y1_args.first, y1_args.second) != 1)
    //     return unexpected(u"Exactly one argument required: y1"_s);
    // if (distance(x2_args.first, x2_args.second) != 1)
    //     return unexpected(u"Exactly one argument required: x2"_s);
    // if (distance(y2_args.first, y2_args.second) != 1)
    //     return unexpected(u"Exactly one argument required: y2"_s);

    // bool ok = false;
    // const auto x1 = args.kw.equal_range(u"x1"_s).first->second.toDouble(&ok);
    // if (!ok)
    //     return unexpected(u"Invalid value for x1: %1"_s.arg(x1_args.first->second));
    // const auto y1 = args.kw.equal_range(u"y1"_s).first->second.toDouble(&ok);
    // if (!ok)
    //     return unexpected(u"Invalid value for y1: %1"_s.arg(y1_args.first->second));
    // const auto x2 = args.kw.equal_range(u"x2"_s).first->second.toDouble(&ok);
    // if (!ok)
    //     return unexpected(u"Invalid value for x2: %1"_s.arg(x2_args.first->second));
    // const auto y2 = args.kw.equal_range(u"y2"_s).first->second.toDouble(&ok);
    // if (!ok)
    //     return unexpected(u"Invalid value for y2: %1"_s.arg(y2_args.first->second));

    // QLinearGradient lg(x1, y1, x2, y2);

    // for (const auto &stops = args.kw.equal_range(u"stop"_s);
    //      const auto &stop : ranges::subrange(stops.first, stops.second) | views::values)

    //     if (const auto stop_args = stop.split(u" "_s, Qt::SkipEmptyParts); stop_args.size() != 2)
    //         return unexpected(u"Gradient stop requires two arguments: %1"_s.arg(stop));

    //     else if (const auto exp_float = parse<double>(stop_args[0]); !exp_float.has_value())
    //         return unexpected(u"Invalid stop value: %1 (%2)"_s.arg(stop_args[0], exp_float.error()));

    //     else if (auto exp_color = parseValue<QColor>(stop_args[1]); !exp_color.has_value())
    //         return unexpected(u"Invalid stop color: %1 "_s.arg(exp_color.error()));

    //     else
    //         lg.setColorAt(*exp_float, *exp_color);

    // lg.setCoordinateMode(QGradient::ObjectMode);

    // return lg;
}

expected<QBrush, QString> StyleFileParser::fnImage(const Fn::Args &args) const
{
    filesystem::path img_path = args.kw.equal_range(u"src"_s).first->second.toStdString();
    if (!img_path.is_absolute())
        img_path = path.parent_path() / img_path;
    auto img = QImage(toQString(img_path));
    img.setDevicePixelRatio(2.0);  // Assume hidpi to allow for seamless dpr transitions
    return QBrush(img);
}

static expected<int, QString> parseIntOrScalar(const QString &cc, int factor)
{
    if (cc.contains(u'.'))
    {
        if (auto exp_scalar = parse<double>(cc))
            return static_cast<int>(lround(*exp_scalar * factor));
        else
            return unexpected(u"Invalid color component: '%1'. %2"_s.arg(cc, exp_scalar.error()));
    }
    else
    {
        if (auto exp_i = parse<int>(cc))
            return *exp_i;
        else
            return unexpected(u"Invalid color component: '%1'. %2"_s.arg(cc, exp_i.error()));
    }
}

expected<QColor, QString> StyleFileParser::fnChange(const Fn::Args &args) const
{
    if (args.pos.size() != 1)
        return unexpected(u"change: Expects exactly one postional argument (color)."_s);

    const auto exp_color = parseResolved<QColor>(args.pos[0]);
    if (!exp_color)
        return unexpected(u"change: Invalid color: %1"_s.arg(exp_color.error()));

    optional<int> red, blue, green, alpha, hue, saturation, lightness;

    const array supported_params {
        tuple(u"red"_s, &red, 255),
        tuple(u"green"_s, &green, 255),
        tuple(u"blue"_s, &blue, 255),
        tuple(u"alpha"_s, &alpha, 255),
        tuple(u"hue"_s, &hue, 359),
        tuple(u"saturation"_s, &saturation, 255),
        tuple(u"lightness"_s, &lightness, 255)
    };

    for (const auto &[param, arg] : args.kw)
    {
        for (const auto &[supported_param, val_ptr, range] : supported_params)
        {
            if (param == supported_param)
            {
                if (auto exp_arg = parseIntOrScalar(arg, range))
                {
                    *val_ptr = *exp_arg;
                    goto found;
                }
                else
                    return unexpected(u"change: Invalid argument '%1' for param '%2': %3"_s
                                          .arg(arg, param, exp_arg.error()));
            }
        }

        return unexpected(u"change: Invalid parameter: %1"_s.arg(param));
    found:
    }

    return change(*exp_color, red, green, blue, alpha, hue, saturation, lightness);
}

expected<QColor, QString> StyleFileParser::fnAdjust(const Fn::Args &args) const
{
    if (args.pos.size() != 1)
        return unexpected(u"adjust: Expects exactly one postional argument (color)."_s);

    const auto exp_color = parseResolved<QColor>(args.pos[0]);
    if (!exp_color)
        return unexpected(u"adjust: Invalid color: %1"_s.arg(exp_color.error()));

    optional<int> red, blue, green, alpha, hue, saturation, lightness;

    const array supported_params {
        tuple(u"red"_s, &red, 255),
        tuple(u"green"_s, &green, 255),
        tuple(u"blue"_s, &blue, 255),
        tuple(u"alpha"_s, &alpha, 255),
        tuple(u"hue"_s, &hue, 359),
        tuple(u"saturation"_s, &saturation, 255),
        tuple(u"lightness"_s, &lightness, 255)
    };

    for (const auto &[param, arg] : args.kw)
    {
        for (const auto &[supported_param, val_ptr, range] : supported_params)
        {
            if (param == supported_param)
            {
                if (auto exp_arg = parseIntOrScalar(arg, range))
                {
                    *val_ptr = *exp_arg;
                    goto found;
                }
                else
                    return unexpected(u"adjust: Invalid argument '%1' for param '%2': %3"_s
                                          .arg(arg, param, exp_arg.error()));
            }
        }

        return unexpected(u"adjust: Invalid parameter: %1"_s.arg(param));
    found:
    }

    return adjust(*exp_color, red, green, blue, alpha, hue, saturation, lightness);
}

expected<QColor, QString> StyleFileParser::fnScale(const Fn::Args &args) const
{
    if (args.pos.size() != 1)
        return unexpected(u"scale: Expects exactly one postional argument (color)."_s);

    const auto exp_color = parseResolved<QColor>(args.pos[0]);
    if (!exp_color)
        return unexpected(u"scale: Invalid color: %1"_s.arg(exp_color.error()));

    optional<double> red, blue, green, alpha, saturation, lightness;

    const array supported_params {
        tuple(u"red"_s, &red),
        tuple(u"green"_s, &green),
        tuple(u"blue"_s, &blue),
        tuple(u"alpha"_s, &alpha),
        tuple(u"saturation"_s, &saturation),
        tuple(u"lightness"_s, &lightness)
    };

    for (const auto &[param, arg] : args.kw)
    {
        for (const auto &[supported_param, val_ptr] : supported_params)
        {
            if (param == supported_param)
            {
                if (auto exp_arg = parse<double>(arg))
                {
                    *val_ptr = *exp_arg;
                    goto found;
                }
                else
                    return unexpected(u"scale: Invalid argument '%1' for param '%2': %3"_s
                                          .arg(arg, param, exp_arg.error()));
            }
        }

        return unexpected(u"scale: Invalid parameter: %1"_s.arg(param));
    found:
    }

    return scale(*exp_color, red, green, blue, alpha, saturation, lightness);
}

expected<QColor, QString> StyleFileParser::fnMultiply(const Fn::Args &args) const
{
    if (args.pos.size() != 1)
        return unexpected(u"multiply: Expects exactly one postional argument (color)."_s);

    const auto exp_color = parseResolved<QColor>(args.pos[0]);
    if (!exp_color)
        return unexpected(u"multiply: Invalid color: %1"_s.arg(exp_color.error()));

    optional<double> red, blue, green, alpha, saturation, lightness;

    const array supported_params {
        tuple(u"red"_s, &red),
        tuple(u"green"_s, &green),
        tuple(u"blue"_s, &blue),
        tuple(u"alpha"_s, &alpha),
        tuple(u"saturation"_s, &saturation),
        tuple(u"lightness"_s, &lightness)
    };

    for (const auto &[param, arg] : args.kw)
    {
        for (const auto &[supported_param, val_ptr] : supported_params)
        {
            if (param == supported_param)
            {
                if (auto exp_arg = parse<double>(arg))
                {
                    *val_ptr = *exp_arg;
                    goto found;
                }
                else
                    return unexpected(u"multiply: Invalid argument '%1' for param '%2': %3"_s
                                          .arg(arg, param, exp_arg.error()));
            }
        }

        return unexpected(u"multiply: Invalid parameter: %1"_s.arg(param));
    found:
    }

    return multiply(*exp_color, red, green, blue, alpha, saturation, lightness);
}

template <>
expected<QBrush, QString>
StyleFileParser::parse(const QString &v) const
{
    if (const auto opt_fn = tryParseFunction(v))
    {
        const auto exp_fn = *opt_fn;
        if (!exp_fn)
            return unexpected(u"Invalid brush function: %1"_s.arg(exp_fn.error()));

        if (const auto it = brush_function_map.find(exp_fn->name);
            it != brush_function_map.end())
            return (this->*it->second)(exp_fn->args);

        if (const auto it = color_function_map.find(exp_fn->name);
            it != color_function_map.end())
            return (this->*it->second)(exp_fn->args);

        return unexpected(u"Invalid brush function: %1"_s.arg(v));
    }
    else if (auto c = QColor(v); c.isValid())
        return c;
    else
        return unexpected(u"Invalid brush function: %1"_s.arg(v));
}

template <>
expected<QColor, QString>
StyleFileParser::parse(const QString &v) const
{
    if (const auto opt_fn = tryParseFunction(v))
    {
        const auto exp_fn = *opt_fn;
        if (!exp_fn)
            return unexpected(u"Invalid function: %1"_s.arg(exp_fn.error()));

        if (const auto it = color_function_map.find(exp_fn->name);
            it != color_function_map.end())
            return (this->*it->second)(exp_fn->args);

        else
            return unexpected(u"Invalid color function: %1"_s.arg(v));
    }
    if (QColor c(v); !c.isValid())
        return unexpected(u"Invalid color: %1"_s.arg(v));
    else
        return c;
}
