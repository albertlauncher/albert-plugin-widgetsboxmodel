// Copyright (c) 2023-2024 Manuel Schneider

#pragma once
#include <QAbstractListModel>
#include <albert/query.h>
#include <map>
namespace albert{
class QueryResult;
class QueryResults;
class QueryExecution;
}

enum ItemRoles
{
    TextRole = Qt::DisplayRole,     ///< QString, The text
    IconRole = Qt::DecorationRole,  ///< QIcon, The icon
    IdentifierRole = Qt::UserRole,  ///< QString, The identifier
    SubTextRole,                    ///< QString, The subtext
    InputActionRole,                ///< QString, The tab action text
    ActionsListRole,                ///< QStringList, List of action names
    ActivateActionRole,             ///< only used for setData. Activates items.
    // Dont change these without changing ItemsModel::roleNames
};


class ResultItemsModel : public QAbstractListModel
{
public:

    ResultItemsModel(const albert::QueryResults&);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

protected:

    const albert::QueryResults &query_results;
    mutable std::map<const albert::QueryResult *, QStringList> actions_cache_;

};


class MatchItemsModel : public ResultItemsModel
{
public:
    MatchItemsModel(albert::detail::Query &query);

    bool canFetchMore(const QModelIndex &) const override;
    void fetchMore(const QModelIndex &) override;

protected:

    albert::detail::Query &query;

};
