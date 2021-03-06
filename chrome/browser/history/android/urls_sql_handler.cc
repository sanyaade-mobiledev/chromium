// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/history/android/urls_sql_handler.h"

#include "base/logging.h"
#include "chrome/browser/history/history_database.h"

using base::Time;

namespace history {

namespace {

const BookmarkRow::BookmarkColumnID kInterestingColumns[] = {
    BookmarkRow::URL, BookmarkRow::VISIT_COUNT, BookmarkRow::TITLE,
    BookmarkRow::LAST_VISIT_TIME };

}  // namespace

UrlsSQLHandler::UrlsSQLHandler(HistoryDatabase* history_db)
    : SQLHandler(kInterestingColumns, arraysize(kInterestingColumns)),
      history_db_(history_db) {
}

UrlsSQLHandler:: ~UrlsSQLHandler() {
}

bool UrlsSQLHandler::Insert(BookmarkRow* row) {
  URLRow url_row(row->url());

  URLID id = history_db_->GetRowForURL(row->url(), &url_row);
  if (id) {
    LOG(ERROR) << "AndroidProviderBackend::Insert Urls; url exists.";
    return false; // We already has this row.
  }

  if (row->is_value_set_explicitly(BookmarkRow::TITLE))
    url_row.set_title(row->title());

  if (row->is_value_set_explicitly(BookmarkRow::LAST_VISIT_TIME))
    url_row.set_last_visit(row->last_visit_time());

  if (row->is_value_set_explicitly(BookmarkRow::VISIT_COUNT))
    url_row.set_visit_count(row->visit_count());

  // Adjust the last_visit_time if it not set.
  if (!row->is_value_set_explicitly(BookmarkRow::LAST_VISIT_TIME)) {
    if (row->is_value_set_explicitly(BookmarkRow::CREATED))
      url_row.set_last_visit(row->created());
    else if (row->is_value_set_explicitly(BookmarkRow::VISIT_COUNT) &&
             row->visit_count() > 0)
      url_row.set_last_visit(Time::Now());
  }

  // Adjust the visit_count if it not set.
  if (!row->is_value_set_explicitly(BookmarkRow::VISIT_COUNT)) {
    int visit_count = 0;
    if (row->is_value_set_explicitly(BookmarkRow::CREATED) &&
        row->is_value_set_explicitly(BookmarkRow::LAST_VISIT_TIME) &&
        row->last_visit_time() == row->created()) {
      visit_count = 1;
    } else {
      if (row->is_value_set_explicitly(BookmarkRow::CREATED))
        visit_count++;
      if (row->is_value_set_explicitly(BookmarkRow::LAST_VISIT_TIME))
        visit_count++;
    }
    url_row.set_visit_count(visit_count);
  }

  URLID new_id = history_db_->AddURL(url_row);

  // The subsequent inserts need this information.
  row->set_url_id(new_id);
  return new_id;
}

// Only the title, the visit time and the vist count can be updated, since the
// visit count and the visit time are related. If they are not both specified,
// The not specified one will be adjusted according the speficied one. The rule
// is:
// a. If the visit time changed and the visit count is not specified, the visit
//    count will be increased by one.
// b. If the visit count increased and the visit time is not specified, the last
//    visit time is set to Now.
// c. If the visit count is 0, it means clear the history, the last visit time
//    will be set to 0.
// d. The new visit time should great than or equal to the current one,
//    otherwise update failed.
// e. The title is free to update.
//
bool UrlsSQLHandler::Update(const BookmarkRow& row,
                            const TableIDRows& ids_set) {
  // Directly updating the URL is not allowed, we should insert the new URL
  // and remove the older one.
  DCHECK(!row.is_value_set_explicitly(BookmarkRow::URL));

  for (TableIDRows::const_iterator ids = ids_set.begin();
       ids != ids_set.end(); ++ids) {
    URLRow url_row;
    if (!history_db_->GetURLRow(ids->url_id, &url_row))
      return false;

    URLRow update_row = url_row;

    if (row.is_value_set_explicitly(BookmarkRow::TITLE))
      update_row.set_title(row.title());

    if (row.is_value_set_explicitly(BookmarkRow::VISIT_COUNT))
      update_row.set_visit_count(row.visit_count());

    if (row.is_value_set_explicitly(BookmarkRow::LAST_VISIT_TIME)) {
      // The new last_visit_time can't be less than current one.
      if (row.last_visit_time() < url_row.last_visit())
        return false;
      update_row.set_last_visit(row.last_visit_time());
    }

    // Adjust the visit_count if it not set.
    if (!row.is_value_set_explicitly(BookmarkRow::VISIT_COUNT) &&
        row.is_value_set_explicitly(BookmarkRow::LAST_VISIT_TIME) &&
        (row.last_visit_time() != url_row.last_visit()))
          // If last visit time is changed, visit count needs increase by 1,
          // as a row will be added in visit database
      update_row.set_visit_count(url_row.visit_count() + 1);

    // Adjust the last_vsit_time if it not set.
    if (!row.is_value_set_explicitly(BookmarkRow::LAST_VISIT_TIME)) {
      if (row.is_value_set_explicitly(BookmarkRow::VISIT_COUNT) &&
          row.visit_count() == 0) {
        // User want to clear history
        update_row.set_last_visit(Time());
      } else if (row.visit_count() > url_row.visit_count()) {
        update_row.set_last_visit(Time::Now());
      }
    }

    if (!history_db_->UpdateURLRow(ids->url_id, update_row))
      return false;
  }
  return true;
}

bool UrlsSQLHandler::Delete(const TableIDRows& ids_set) {
  for (TableIDRows::const_iterator ids = ids_set.begin();
       ids != ids_set.end(); ++ids) {
    if (!history_db_->DeleteURLRow(ids->url_id))
      return false;
  }
  return true;
}

}  // namespace history
