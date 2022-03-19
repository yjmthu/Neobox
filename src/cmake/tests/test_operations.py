#!/usr/bin/env python3
#############################################################################
##
## Copyright (C) 2018 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the plugins of the Qt Toolkit.
##
## $QT_BEGIN_LICENSE:GPL-EXCEPT$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 3 as published by the Free Software
## Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

from pro2cmake import AddOperation, SetOperation, UniqueAddOperation, RemoveOperation

def test_add_operation():
    op = AddOperation(['bar', 'buz'])

    result = op.process(['foo', 'bar'], ['foo', 'bar'], lambda x: x)
    assert ['foo', 'bar', 'bar', 'buz'] == result


def test_uniqueadd_operation():
    op = UniqueAddOperation(['bar', 'buz'])

    result = op.process(['foo', 'bar'], ['foo', 'bar'], lambda x: x)
    assert ['foo', 'bar', 'buz'] == result


def test_set_operation():
    op = SetOperation(['bar', 'buz'])

    result = op.process(['foo', 'bar'], ['foo', 'bar'], lambda x: x)
    assert ['bar', 'buz'] == result


def test_remove_operation():
    op = RemoveOperation(['bar', 'buz'])

    result = op.process(['foo', 'bar'], ['foo', 'bar'], lambda x: x)
    assert ['foo', '-buz'] == result
