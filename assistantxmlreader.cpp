#include "assistantxmlreader.h"
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDebug>

namespace {
const QString ROOT_TAG = "assistants";
const QString ASSISTANT_TAG = "assistant";
}

AssistantXmlReader::AssistantXmlReader(QObject *parent)
    : QObject(parent)
{
}

bool AssistantXmlReader::readFile(const QString &path)
{
    foreach(Assistant* assistant, m_items) {
        delete assistant;
    }
    m_items.clear();

    QDir dir = QFileInfo(path).absoluteDir();
    if (dir.cd("icons")) {
        m_iconDir = dir.absolutePath();
    } else {
        m_iconDir.clear();
    }
    qDebug() << "icon path:" << m_iconDir;

    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "can't read assistant file:" << path;
        qDebug() << "i/o error:" << file.errorString();
        return false;
    }
    m_reader.setDevice(&file);
    m_reader.readNext();
    while (!m_reader.atEnd()) {
        if (m_reader.isStartElement()) {
            if (m_reader.name() == ROOT_TAG) {
                readRootElement();
            } else {
                m_reader.raiseError(QObject::tr("Not an assistants XML file"));
            }
        } else {
            m_reader.readNext();
        }
    }

    file.close();
    if (m_reader.hasError()) {
        qDebug() << "failed to parse assistant file:" << path;
        qDebug() << "xml parsing error:" << m_reader.errorString();
        return false;
    } else if (file.error() != QFile::NoError) {
        qDebug() << "can't read assistant file:" << path;
        qDebug() << "i/o error:" << file.errorString();
        return false;
    }

    return true;
}

void AssistantXmlReader::readRootElement()
{
    m_reader.readNext();
    while (!m_reader.atEnd()) {
        if (m_reader.isEndElement()) {
            m_reader.readNext();
            break;
        }

        if (m_reader.isStartElement()) {
            if (m_reader.name() == ASSISTANT_TAG) {
                readAssistantElement();
            } else {
                skipUnknownElement();
            }
        } else {
            m_reader.readNext();
        }
    }
}

void AssistantXmlReader::skipUnknownElement()
{
    m_reader.readNext();
    while (!m_reader.atEnd()) {
        if (m_reader.isEndElement()) {
            m_reader.readNext();
            break;
        }
        if (m_reader.isStartElement()) {
            skipUnknownElement();
        } else {
            m_reader.readNext();
        }
    }
}

void AssistantXmlReader::readAssistantElement()
{
    Assistant* assistant = new Assistant(m_reader.attributes().value("name").toString(), this);
    m_items.append(assistant);
    skipUnknownElement();
}

AssistantItem::AssistantItem(const QString &name, const QString &data, Assistant *parent)
    : QObject(parent)
    , m_name(name)
    , m_data(data)
{
}


Assistant::Assistant(const QString &name, AssistantXmlReader *parent)
    : QObject(parent)
    , m_name(name)
{
}
