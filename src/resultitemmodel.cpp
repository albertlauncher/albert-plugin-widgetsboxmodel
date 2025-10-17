// Copyright (c) 2022-2025 Manuel Schneider

#include "resultitemmodel.h"
#include <QIcon>
#include <QStringListModel>
#include <QTimer>
#include <albert/extension.h>
#include <albert/frontend.h>
#include <albert/iconutil.h>
#include <albert/item.h>
#include <albert/logging.h>
#include <albert/query.h>
#include <albert/queryexecution.h>
#include <albert/queryhandler.h>
#include <albert/queryresults.h>
#include <albert/rankitem.h>
using enum ItemRoles;
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

ResultItemsModel::ResultItemsModel(const QueryResults &r) :
    query_results(r)
{
    connect(&query_results, &QueryResults::resultsAboutToBeInserted,
            this, [this](int first, int last)
            { beginInsertRows({}, first, last); });

    connect(&query_results, &QueryResults::resultsInserted,
            this, [this]
            { endInsertRows(); });

    connect(&query_results, &QueryResults::resultsAboutToBeRemoved,
            this, [this](int first, int last)
            { beginRemoveRows({}, first, last); });

    connect(&query_results, &QueryResults::resultsRemoved,
            this, [this]
            { endRemoveRows(); });

    connect(&query_results, &QueryResults::resultsAboutToBeMoved,
            this, [this](int srcFirst, int srcLast, int dst)
            { beginMoveRows({}, srcFirst, srcLast, {}, dst); });

    connect(&query_results, &QueryResults::resultsMoved,
            this, [this]
            { endMoveRows(); });

    connect(&query_results, &QueryResults::resultsAboutToBeReset,
            this, [this]
            { beginResetModel(); });

    connect(&query_results, &QueryResults::resultsReset,
            this, [this]
            { endResetModel(); });

    connect(&query_results, &QueryResults::resultChanged,
            this, [this](uint idx)
            { emit dataChanged(index(idx, 0), index(idx, 0)); });
}

int ResultItemsModel::rowCount(const QModelIndex &) const { return (int) query_results.count(); }

QVariant ResultItemsModel::data(const QModelIndex &index, int role) const
{
    const auto &query_result = query_results[index.row()];
    const auto &[extension, item] = query_result;

    switch (role) {

    case IdentifierRole:
    {
        try {
            return u"%1.%2"_s.arg(extension->id(), item->id());
        } catch (const exception &e) {
            WARN << "Exception in Item::id:" << e.what();
        }
        return {};
    }

    case TextRole:
    {
        try {
            auto text = item->text();
            text.replace(u'\n', u' ');
            return text;
        } catch (const exception &e) {
            WARN << "Exception in Item::text:" << e.what();
        }
        return {};
    }

    case SubTextRole:
    {
        try {
            auto text = item->subtext();
            text.replace(u'\n', u' ');
            return text;
        } catch (const exception &e) {
            WARN << "Exception in Item::subtext:" << e.what();
        }
        return {};
    }

    case Qt::ToolTipRole:
    {
        try {
            const auto text = item->text();
            try {
                return u"%1\n%2"_s.arg(text, item->subtext());
            } catch (const exception &e) {
                WARN << "Exception in Item::subtext:" << e.what();
            }
        } catch (const exception &e) {
            WARN << "Exception in Item::text:" << e.what();
        }
        return {};
    }

    case InputActionRole:
    {
        try {
            return item->inputActionText();
        } catch (const exception &e) {
            WARN << "Exception in Item::inputActionText:" << e.what();
        }
        return {};
    }

    case IconRole:
    {
        try {
            return qIcon(item->icon());
        } catch (const exception &e) {
            WARN << "Exception in Item::makeIcon:" << e.what();
        }
        return {};
    }

    case ActionsListRole:
    {
        if (auto it = actions_cache_.find(&query_result);
            it != actions_cache_.end())
            return it->second;

        try {
            QStringList action_names;
            for (const auto &action : item->actions())
                action_names << action.text;
            // actions_cache_.emplace(make_pair(extension, item.get()), actions_cache_);
            return action_names;
        } catch (const exception &e) {
            WARN << "Exception in Item::actions:" << e.what();
        }
        return {};
    }
    }
    return {};
}

MatchItemsModel::MatchItemsModel(const albert::QueryResults &results, albert::QueryExecution &execution)
    : ResultItemsModel(results)
    , query_execution(execution) {}

bool MatchItemsModel::canFetchMore(const QModelIndex &) const
{
    return query_execution.canFetchMore();
}

void MatchItemsModel::fetchMore(const QModelIndex &)
{
    query_execution.fetchMore();
}
