#!/usr/bin/env bash

#############################################################################
##
## Copyright (C) 2021 The Qt Company Ltd.
## Copyright (C) 2017 Pelagicore AG
## Contact: https://www.qt.io/licensing/
##
## This file is part of the provisioning scripts of the Qt Toolkit.
##
## $QT_BEGIN_LICENSE:LGPL$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU Lesser General Public License Usage
## Alternatively, this file may be used under the terms of the GNU Lesser
## General Public License version 3 as published by the Free Software
## Foundation and appearing in the file LICENSE.LGPL3 included in the
## packaging of this file. Please review the following information to
## ensure the GNU Lesser General Public License version 3 requirements
## will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 2.0 or (at your option) the GNU General
## Public license version 3 or any later version approved by the KDE Free
## Qt Foundation. The licenses are as published by the Free Software
## Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-2.0.html and
## https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

# This script installs python3

# shellcheck source=../unix/SetEnvVar.sh
source "${BASH_SOURCE%/*}/../common/unix/SetEnvVar.sh"

brew install --formula ${BASH_SOURCE%/*}/pyenv.rb

pyenv install 3.9.7

/Users/qt/.pyenv/versions/3.9.7/bin/pip3 install --user install virtualenv wheel html5lib

SetEnvVar "PYTHON3_PATH" "/Users/qt/.pyenv/versions/3.9.7/bin/"
SetEnvVar "PIP3_PATH" "/Users/qt/.pyenv/versions/3.9.7/bin/"
# Use 3.9 as a default python
SetEnvVar "PATH" "\$PYTHON3_PATH:\$PATH"

# Install all needed packages in a special wheel cache directory
/Users/qt/.pyenv/versions/3.9.7/bin/pip3 wheel --wheel-dir $HOME/python3-wheels -r ${BASH_SOURCE%/*}/../common/shared/requirements.txt
SetEnvVar "PYTHON3_WHEEL_CACHE" "$HOME/python3-wheels"

echo "python3 = 3.9.7" >> ~/versions.txt
