/*
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <http://unlicense.org>
 */

#ifndef MINISAXCPP_XMLITEMPARSER_H
#define MINISAXCPP_XMLITEMPARSER_H

#include "UnicodeCircularBuffer.h"
#include "Common.h"

namespace MiniSaxCpp
{
/**
 * XML Item Parser class can be used to parse a individual XML items
 *
 * Supported XML items:
 *  - Start of XML item
 *  - End of XML item
 *  - XML item type
 *  - Name
 *  - PI's value
 *  - TODO: add others
 */
class XmlItemParser
{
public:
    // Public types
    enum Result
    {
        Result_NeedMoreData,
        Result_Success,
        Result_Error
    };

    enum Option
    {
        Option_None,
        Option_Synchronization,
        Option_IgnoreLeadingWhitespace
    };

    enum ItemType
    {
        ItemType_None,
        ItemType_Whitespace,
        ItemType_ProcessingInstruction,
        ItemType_DocumentType,
        ItemType_Comment,
        ItemType_StartOfElement
    };

    enum Action
    {
        Action_ReadItem,
        Action_ReadName,
        Action_ReadPiValue,
        Action_ReadDocumentTypeValue,
        Action_ReadCommentText,
        Action_ReadElementStartOfContent,
        Action_ReadElementEndEmpty,
        Action_ReadAttributeValue
    };

public:
    XmlItemParser(const size_t dataBufferSize);

    void clear();
    bool writeData(const char data);

    bool configure(const Action action, const Option option = Option_None);
    Result execute();

    uint32_t getTerminationCharacter();
    ItemType getItemType() const;
    UnicodeString getValue() const;

private:
    // Private types
    enum State
    {
        State_Idle,
        State_WaitingForStartOfItem,
        State_WhitespaceRead,
        State_ReadingItemType,
        State_ItemTypeRead,

        State_ReadingName,
        State_NameRead,

        State_ReadingPiValue,
        State_PiValueRead,

        State_ReadingDocumentTypeValue,
        State_DocumentTypeValueRead,

        State_ReadingCommentText,
        State_CommentTextRead,

        State_ReadingElementStartOfContent,
        State_ElementStartOfContentRead,

        State_ReadingElementEndEmpty,
        State_ElementEndEmptyRead,

        State_ReadingAttributeValueEqual,
        State_ReadingAttributeValueQuote,
        State_ReadingAttributeValueContent,
        State_AttributeValueRead,

        State_Error
    };

private:
    // Private API
    void eraseFromParsingBuffer(const size_t size);
    bool readData();
    bool readDataIfNeeded();

    bool checkActionReadItem(Option option);
    State executeStateWaitingForStartOfItem();
    State executeStateReadingItemType();

    bool checkActionReadName(Option option);
    State executeStateReadingName();

    bool checkActionReadPiValue(Option option);
    State executeStateReadingPiValue();

    bool checkActionReadDocumentTypeValue(Option option);
    State executeStateReadingDocumentTypeValue();

    bool checkActionReadCommentText(Option option);
    State executeStateReadingCommentText();

    bool checkActionReadElementStartOfContent(Option option);
    State executeStateReadingElementStartOfContent();

    bool checkActionReadElementEndEmpty(Option option);
    State executeStateReadingElementEndEmpty();

    bool checkActionReadAttributeValue(Option option);
    State executeStateReadingAttributeValueEqual();
    State executeStateReadingAttributeValueQuote();
    State executeStateReadingAttributeValueContent();

private:
    // Private data
    UnicodeCircularBuffer m_xmlDataBuffer;
    UnicodeString m_parsingBuffer;
    size_t m_position;

    Option m_option;
    State m_state;
    uint32_t m_terminationCharacter;
    ItemType m_itemType;
    UnicodeString m_value;
    Common::QuotationMark m_quotationMark;
};
}

#endif // MINISAXCPP_XMLITEMPARSER_H
