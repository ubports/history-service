/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of history-service.
 *
 * history-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * history-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "phoneutils_p.h"
#include "phonenumberutils_p.h"

PhoneUtils::PhoneUtils(QObject *parent) :
    QObject(parent)
{
}

bool PhoneUtils::comparePhoneNumbers(const QString &number1, const QString &number2)
{
    if (isPhoneNumber(number1) && isPhoneNumber(number2)) {
        return PhoneNumberUtils::compareLoosely(number1, number2);
    }
    return number1 == number2;
}

bool PhoneUtils::isSameContact(const QString &a, const QString &b)
{
    if (isPhoneNumber(a) && isPhoneNumber(b)) {
        return PhoneNumberUtils::compareLoosely(a, b);
    }
    // if at least one of the id's is not a phone number, then perform a simple string comparison
    return a == b;
}

bool PhoneUtils::isPhoneNumber(const QString &identifier) {
    // remove all non diable digits
    QString finalNumber = QString(identifier).replace(QRegExp("[p+*#(),;-]"),"");
    finalNumber = finalNumber.replace(QRegExp("(\\s+)"), "");
    // if empty, the number is invalid
    if (finalNumber.isEmpty())
        return false;

    finalNumber = finalNumber.replace(QRegExp("(\\d+)"), "");
    return finalNumber.isEmpty();
}


