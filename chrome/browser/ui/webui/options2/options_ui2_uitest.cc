// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/options2/options_ui2_uitest.h"

#include "base/string16.h"
#include "base/utf_string_conversions.h"
#include "chrome/common/url_constants.h"
#include "chrome/test/automation/automation_proxy.h"
#include "chrome/test/automation/browser_proxy.h"
#include "chrome/test/automation/tab_proxy.h"
#include "grit/generated_resources.h"
#include "ui/base/l10n/l10n_util.h"

namespace options2 {

OptionsUITest::OptionsUITest() {
  dom_automation_enabled_ = true;
}

void OptionsUITest::NavigateToSettings(scoped_refptr<TabProxy> tab) {
  const GURL& url = GURL(chrome::kChromeUISettingsURL);
  ASSERT_EQ(AUTOMATION_MSG_NAVIGATION_SUCCESS,
      tab->NavigateToURLBlockUntilNavigationsComplete(url, 1)) << url.spec();
}

void OptionsUITest::VerifyNavbar(scoped_refptr<TabProxy> tab) {
  bool navbar_exist = false;
  EXPECT_TRUE(tab->ExecuteAndExtractBool(L"",
      L"domAutomationController.send("
      L"!!document.getElementById('navigation'))", &navbar_exist));
  EXPECT_EQ(true, navbar_exist);
}

void OptionsUITest::VerifyTitle(scoped_refptr<TabProxy> tab) {
  std::wstring title;
  EXPECT_TRUE(tab->GetTabTitle(&title));
  string16 expected_title = l10n_util::GetStringUTF16(IDS_SETTINGS_TITLE);
  EXPECT_NE(WideToUTF16Hack(title).find(expected_title), string16::npos);
}

// TODO(estade): this is failing on the bots: http://crbug.com/119671
TEST_F(OptionsUITest, DISABLED_LoadOptionsByURL) {
  scoped_refptr<BrowserProxy> browser(automation()->GetBrowserWindow(0));
  ASSERT_TRUE(browser.get());

  scoped_refptr<TabProxy> tab = browser->GetActiveTab();
  ASSERT_TRUE(tab.get());

  NavigateToSettings(tab);
  VerifyTitle(tab);
  VerifyNavbar(tab);
}

}  // namespace options2
