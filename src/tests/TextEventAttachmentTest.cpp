/*
 * Copyright (C) 2013 Canonical, Ltd.
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

#include <QtCore/QObject>
#include <QtTest/QtTest>

#include "thread.h"
#include "textevent.h"
#include "voiceevent.h"

Q_DECLARE_METATYPE(History::AttachmentFlags)
Q_DECLARE_METATYPE(History::AttachmentFlag)

class TextEventAttachmentTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testCreateNewTextEventAttachment_data();
    void testCreateNewTextEventAttachment();
    void testFromProperties();
    void testCopyConstructor();
    void testAssignment();
    void testEquals_data();
    void testEquals();
    void testIsNull();
};

void TextEventAttachmentTest::initTestCase()
{
    qRegisterMetaType<History::AttachmentFlags>();
    qRegisterMetaType<History::AttachmentFlag>();
}

void TextEventAttachmentTest::testCreateNewTextEventAttachment_data()
{
    QTest::addColumn<QString>("accountId");
    QTest::addColumn<QString>("threadId");
    QTest::addColumn<QString>("eventId");
    QTest::addColumn<QString>("attachmentId");
    QTest::addColumn<QString>("contentType");
    QTest::addColumn<QString>("filePath");
    QTest::addColumn<History::AttachmentFlags>("status");

    QTest::newRow("regular attachment")
            << "someaccountid" << "somethreadid" << "someeventid"
            << "someattachmentid" << "image/x-jpeg" << "/some/file/path.jpg" << History::AttachmentFlags(History::AttachmentDownloaded);
    QTest::newRow("no content type and pending attachment")
            << "anotheraccountid" << "anotherthreadid" << "anothereventid"
            << "anotherattachmentid" << "" << "/another/file/path.jpg" << History::AttachmentFlags(History::AttachmentPending);
    QTest::newRow("attachment with error")
            << "yetanotheraccountid" << "yetanotherthreadid" << "yetanothereventid"
            << "yetanotherattachmentid" << "" << "/some/file/path" << History::AttachmentFlags(History::AttachmentError);
}

void TextEventAttachmentTest::testCreateNewTextEventAttachment()
{
    QFETCH(QString, accountId);
    QFETCH(QString, threadId);
    QFETCH(QString, eventId);
    QFETCH(QString, attachmentId);
    QFETCH(QString, contentType);
    QFETCH(QString, filePath);
    QFETCH(History::AttachmentFlags, status);

    History::TextEventAttachment attachment(accountId, threadId, eventId,
                                            attachmentId, contentType, filePath, status);
    QCOMPARE(attachment.accountId(), accountId);
    QCOMPARE(attachment.threadId(), threadId);
    QCOMPARE(attachment.eventId(), eventId);
    QCOMPARE(attachment.attachmentId(), attachmentId);
    QCOMPARE(attachment.contentType(), contentType);
    QCOMPARE(attachment.filePath(), filePath);
    QCOMPARE(attachment.status(), status);

    QVariantMap properties = attachment.properties();
    QCOMPARE(properties[History::FieldAccountId].toString(), accountId);
    QCOMPARE(properties[History::FieldThreadId].toString(), threadId);
    QCOMPARE(properties[History::FieldEventId].toString(), eventId);
    QCOMPARE(properties[History::FieldAttachmentId].toString(), attachmentId);
    QCOMPARE(properties[History::FieldContentType].toString(), contentType);
    QCOMPARE(properties[History::FieldFilePath].toString(), filePath);
    QCOMPARE(properties[History::FieldStatus].toInt(), (int) status);
}

void TextEventAttachmentTest::testFromProperties()
{
    QVariantMap properties;
    properties[History::FieldAccountId] = "someAccountId";
    properties[History::FieldThreadId] = "someThreadId";
    properties[History::FieldEventId] = "someEventId";
    properties[History::FieldAttachmentId] = "someAttachmentId";
    properties[History::FieldContentType] = "someContentType";
    properties[History::FieldFilePath] = "/some/file/path";
    properties[History::FieldStatus] = (int) History::AttachmentDownloaded;

    History::TextEventAttachment attachment = History::TextEventAttachment::fromProperties(properties);
    QCOMPARE(attachment.accountId(), properties[History::FieldAccountId].toString());
    QCOMPARE(attachment.threadId(), properties[History::FieldThreadId].toString());
    QCOMPARE(attachment.eventId(), properties[History::FieldEventId].toString());
    QCOMPARE(attachment.attachmentId(), properties[History::FieldAttachmentId].toString());
    QCOMPARE(attachment.contentType(), properties[History::FieldContentType].toString());
    QCOMPARE(attachment.filePath(), properties[History::FieldFilePath].toString());
    QCOMPARE(attachment.status(), (History::AttachmentFlags) properties[History::FieldStatus].toInt());

    // now load from an empty map
    History::TextEventAttachment emptyAttachment = History::TextEventAttachment::fromProperties(QVariantMap());
    QVERIFY(emptyAttachment.isNull());
}

void TextEventAttachmentTest::testCopyConstructor()
{
    History::TextEventAttachment attachment("oneAccountId", "oneThreadId", "oneEventId",
                                            "oneAttachmentId", "oneContentType", "/one/file/path", History::AttachmentPending);
    History::TextEventAttachment copy(attachment);

    QCOMPARE(copy.accountId(), attachment.accountId());
    QCOMPARE(copy.threadId(), attachment.threadId());
    QCOMPARE(copy.eventId(), attachment.eventId());
    QCOMPARE(copy.attachmentId(), attachment.attachmentId());
    QCOMPARE(copy.contentType(), attachment.contentType());
    QCOMPARE(copy.filePath(), attachment.filePath());
    QCOMPARE(copy.status(), attachment.status());
}

void TextEventAttachmentTest::testAssignment()
{
    History::TextEventAttachment attachment("oneAccountId", "oneThreadId", "oneEventId",
                                            "oneAttachmentId", "oneContentType", "/one/file/path", History::AttachmentPending);
    History::TextEventAttachment copy;
    copy = attachment;
    QCOMPARE(copy.accountId(), attachment.accountId());
    QCOMPARE(copy.threadId(), attachment.threadId());
    QCOMPARE(copy.eventId(), attachment.eventId());
    QCOMPARE(copy.attachmentId(), attachment.attachmentId());
    QCOMPARE(copy.contentType(), attachment.contentType());
    QCOMPARE(copy.filePath(), attachment.filePath());
    QCOMPARE(copy.status(), attachment.status());
}

void TextEventAttachmentTest::testEquals_data()
{
    QTest::addColumn<QString>("accountId");
    QTest::addColumn<QString>("threadId");
    QTest::addColumn<QString>("eventId");
    QTest::addColumn<QString>("attachmentId");
    QTest::addColumn<QString>("secondAccountId");
    QTest::addColumn<QString>("secondThreadId");
    QTest::addColumn<QString>("secondEventId");
    QTest::addColumn<QString>("secondAttachmentId");
    QTest::addColumn<bool>("result");

    QTest::newRow("equal")
            << "someaccountid" << "somethreadid" << "someeventid" << "someattachmentid"
            << "someaccountid" << "somethreadid" << "someeventid" << "someattachmentid" << true;
    QTest::newRow("different accountId")
            << "someaccountid" << "somethreadid" << "someeventid" << "someattachmentid"
            << "otheraccountid" << "somethreadid" << "someeventid" << "someattachmentid" << false;
    QTest::newRow("different threadId")
            << "someaccountid" << "somethreadid" << "someeventid" << "someattachmentid"
            << "someaccountid" << "otherthreadid" << "someeventid" << "someattachmentid" << false;
    QTest::newRow("different eventId")
            << "someaccountid" << "somethreadid" << "someeventid" << "someattachmentid"
            << "someaccountid" << "somethreadid" << "othereventid" << "someattachmentid" << false;
    QTest::newRow("different attachmentId")
            << "someaccountid" << "somethreadid" << "someeventid" << "someattachmentid"
            << "someaccountid" << "somethreadid" << "someeventid" << "otherattachmentid" << false;
}

void TextEventAttachmentTest::testEquals()
{
    QFETCH(QString, accountId);
    QFETCH(QString, threadId);
    QFETCH(QString, eventId);
    QFETCH(QString, attachmentId);
    QFETCH(QString, secondAccountId);
    QFETCH(QString, secondThreadId);
    QFETCH(QString, secondEventId);
    QFETCH(QString, secondAttachmentId);
    QFETCH(bool, result);


    History::TextEventAttachment attachment(accountId, threadId, eventId, attachmentId,
                                            "oneContentType", "/one/file/path", History::AttachmentPending);
    History::TextEventAttachment anotherAttachment(secondAccountId, secondThreadId, secondEventId, secondAttachmentId,
                                                   "anotherContentType", "/different/file/path", History::AttachmentDownloaded);
    QCOMPARE(attachment == anotherAttachment, result);
}

void TextEventAttachmentTest::testIsNull()
{
    History::TextEventAttachment attachment;
    QVERIFY(attachment.isNull());
}

QTEST_MAIN(TextEventAttachmentTest)
#include "TextEventAttachmentTest.moc"
