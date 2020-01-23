/*
 * Copyright (C) 2019-2020 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include "table_list.h"
#include <QtWidgets>

TableList::TableList()
{
  const char *style =
      "QTableWidget { background-color: #e0e0e0; color: black; } "
      "QLineEdit { background:white; } "
      "QCheckBox { padding-left: 5px; background:white; } "
      "QPushButton { margin: 5px; background-color: #c0c0c0; border: 1px solid black; } "
      "QPushButton:pressed { background-color: #808080; }";
  setStyleSheet(style);
  setColumnCount(2);
  setMinimumSize(400, 200);
  setSizePolicy(
      QSizePolicy::Fixed,
      QSizePolicy::MinimumExpanding);
  horizontalHeader()->setVisible(false);
  verticalHeader()->setVisible(false);
  horizontalHeader()->setSectionResizeMode(
      0, QHeaderView::Stretch);
  horizontalHeader()->setSectionResizeMode(
      1, QHeaderView::ResizeToContents);
  verticalHeader()->setSectionResizeMode(
      QHeaderView::ResizeToContents);
  setAutoFillBackground(true);
}

TableList::~TableList()
{
}
