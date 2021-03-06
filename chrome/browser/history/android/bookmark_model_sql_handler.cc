// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/history/android/bookmark_model_sql_handler.h"

#include "base/logging.h"
#include "chrome/browser/bookmarks/bookmark_model.h"
#include "chrome/browser/bookmarks/bookmark_service.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/history/url_database.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "content/public/browser/browser_thread.h"

using base::Time;
using content::BrowserThread;

namespace history {

namespace {

// The interesting columns of this handler.
const BookmarkRow::BookmarkColumnID kInterestingColumns[] = {
  BookmarkRow::BOOKMARK, BookmarkRow::TITLE };

} // namespace

BookmarkModelSQLHandler::Task::Task(Profile* profile)
    : profile_(profile) {
}

void BookmarkModelSQLHandler::Task::AddBookmarkToMobileFolder(
    const GURL& url,
    const string16& title) {
  BookmarkModel* bookmark_model = GetBookmarkModel();
  if (!bookmark_model)
    return;
  const BookmarkNode* mobile_node = bookmark_model->mobile_node();
  if (mobile_node)
    bookmark_model->AddURL(mobile_node, 0, title, url);
}

void BookmarkModelSQLHandler::Task::AddBookmark(const GURL& url,
                                                const string16& title,
                                                int64 parent_id) {
  BookmarkModel* bookmark_model = GetBookmarkModel();
  if (!bookmark_model)
    return;
  const BookmarkNode* parent = bookmark_model->GetNodeByID(parent_id);
  if (parent)
    bookmark_model->AddURL(parent, 0, title, url);
}

void BookmarkModelSQLHandler::Task::RemoveBookmark(const GURL& url) {
  BookmarkModel* bookmark_model = GetBookmarkModel();
  if (!bookmark_model)
    return;
  std::vector<const BookmarkNode*> nodes;
  bookmark_model->GetNodesByURL(url, &nodes);
  for (std::vector<const BookmarkNode*>::iterator i = nodes.begin();
       i != nodes.end(); ++i) {
    const BookmarkNode* parent_node = (*i)->parent();
    bookmark_model->Remove(parent_node, parent_node->GetIndexOf(*i));
  }
}

void BookmarkModelSQLHandler::Task::UpdateBookmarkTitle(const GURL& url,
                                                        const string16&title) {
  BookmarkModel* bookmark_model = GetBookmarkModel();
  if (!bookmark_model)
    return;
  std::vector<const BookmarkNode*> nodes;
  bookmark_model->GetNodesByURL(url, &nodes);
  for (std::vector<const BookmarkNode*>::iterator i = nodes.begin();
       i != nodes.end(); ++i) {
    bookmark_model->SetTitle(*i, title);
  }
}


BookmarkModelSQLHandler::Task::~Task() {
}

BookmarkModel* BookmarkModelSQLHandler::Task::GetBookmarkModel() {
  ProfileManager* manager = g_browser_process->profile_manager();
  if (!manager)
    return NULL;
  if (!manager->IsValidProfile(profile_))
    return NULL;
  return profile_->GetBookmarkModel();
}

BookmarkModelSQLHandler::BookmarkModelSQLHandler(
    URLDatabase* url_database,
    Profile* profile)
    : SQLHandler(kInterestingColumns, arraysize(kInterestingColumns)),
      url_database_(url_database),
      profile_(profile) {
}

BookmarkModelSQLHandler::~BookmarkModelSQLHandler() {
}

bool BookmarkModelSQLHandler::Update(const BookmarkRow& row,
                                     const TableIDRows& ids_set) {
  for (TableIDRows::const_iterator i = ids_set.begin();
      i != ids_set.end(); ++i) {
    if (row.is_value_set_explicitly(BookmarkRow::BOOKMARK)) {
      if (row.is_bookmark()) {
        URLRow url_row;
        if (!url_database_->GetURLRow(i->url_id, &url_row))
          return false;
        BrowserThread::PostTask(BrowserThread::UI, FROM_HERE, base::Bind(
            &BookmarkModelSQLHandler::Task::AddBookmark,
            scoped_refptr<BookmarkModelSQLHandler::Task>(
                new BookmarkModelSQLHandler::Task(profile_)),
            i->url, url_row.title(), row.parent_id()));
      } else {
        BrowserThread::PostTask(BrowserThread::UI, FROM_HERE, base::Bind(
            &BookmarkModelSQLHandler::Task::RemoveBookmark,
            scoped_refptr<BookmarkModelSQLHandler::Task>(
                new BookmarkModelSQLHandler::Task(profile_)),
            i->url));
      }
    } else if (row.is_value_set_explicitly(BookmarkRow::TITLE)) {
      BrowserThread::PostTask(BrowserThread::UI, FROM_HERE, base::Bind(
          &BookmarkModelSQLHandler::Task::UpdateBookmarkTitle,
          scoped_refptr<BookmarkModelSQLHandler::Task>(
              new BookmarkModelSQLHandler::Task(profile_)),
          i->url, row.title()));
    }
  }
  return true;
}

bool BookmarkModelSQLHandler::Delete(const TableIDRows& ids_set) {
  for (TableIDRows::const_iterator i = ids_set.begin();
       i != ids_set.end(); ++i) {
    BrowserThread::PostTask(BrowserThread::UI, FROM_HERE, base::Bind(
        &BookmarkModelSQLHandler::Task::RemoveBookmark,
        scoped_refptr<BookmarkModelSQLHandler::Task>(
            new BookmarkModelSQLHandler::Task(profile_)),
        i->url));
  }
  return true;
}

bool BookmarkModelSQLHandler::Insert(BookmarkRow* row) {
  DCHECK(row->is_value_set_explicitly(BookmarkRow::URL));
  if (row->is_value_set_explicitly(BookmarkRow::PARENT_ID)) {
    BrowserThread::PostTask(BrowserThread::UI, FROM_HERE, base::Bind(
        &BookmarkModelSQLHandler::Task::AddBookmark,
        scoped_refptr<BookmarkModelSQLHandler::Task>(
            new BookmarkModelSQLHandler::Task(profile_)),
        row->url(), row->title(), row->parent_id()));
  } else {
    BrowserThread::PostTask(BrowserThread::UI, FROM_HERE, base::Bind(
        &BookmarkModelSQLHandler::Task::AddBookmarkToMobileFolder,
        scoped_refptr<BookmarkModelSQLHandler::Task>(
            new BookmarkModelSQLHandler::Task(profile_)),
        row->url(), row->title()));
  }
  return true;
}

}  // namespace history.
