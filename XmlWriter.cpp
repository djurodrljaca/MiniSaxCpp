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
#include "XmlWriter.h"
#include "Utf.h"
#include "XmlValidator.h"
#include "Common.h"

using namespace MiniSaxCpp;

/**
 * Constructor
 */
XmlWriter::XmlWriter()
{
    clearDocument();
}

/**
 * Clear XML document
 */
void XmlWriter::clearDocument()
{
    m_state = State_Empty;
    m_xmlDeclarationSet = false;
    m_documentType;
    m_openedElementList.clear();
    m_currentElementInfo = ElementInfo();
    m_attributeNameList.clear();
    m_xmlString.clear();
}

/**
 * Get XML string
 *
 * \return XML string
 * \return Empty string on error
 *
 * \note XML string will only be returned if XML document has been fully completed (root element
 *       has to be ended), otherwise an empty string will be returned.
 */
std::string XmlWriter::getXmlString() const
{
    std::string xmlString;

    if (m_state == State_DocumentEnded)
    {
        xmlString = m_xmlString;
    }

    return xmlString;
}

/**
 * Sets XML Declaration in the XML document
 *
 * \retval true     Success
 * \retval false    Error, XML document is not empty
 *
 * \note Currently only XML Declaration with parameters version 1.0 with encoding "UTF-8" are
 *       supported!
 */
bool XmlWriter::setXmlDeclaration()
{
    bool success = false;

    if (m_state == State_Empty)
    {
        // Set XML Declaration string
        m_xmlString = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

        m_xmlDeclarationSet = true;
        m_state = State_DocumentStarted;
        success = true;
    }

    return success;
}

/**
 * Sets Document Type in the XML document
 *
 * \param documentType  Document type name
 *
 * \retval true     Success
 * \retval false    Error
 *
 * \note If the document type name is set then the root element name must match the document type
 *       name!
 */
bool XmlWriter::setDocumentType(const std::string &documentType)
{
    bool success = false;

    if (m_documentType.empty())
    {
        if ((m_state == State_Empty) ||
            (m_state == State_DocumentStarted))
        {
            if (XmlValidator::validateNameString(documentType))
            {
                // Create Document Type string
                m_xmlString.append("<!DOCTYPE ").append(documentType).append(">");

                m_documentType = documentType;
                m_state = State_DocumentStarted;
                success = true;
            }
        }
    }

    return success;
}

/**
 * Add a comment to the XML document
 *
 * \param commentText   Text that should be written to the comment
 *
 * \retval true     Success
 * \retval false    Error
 */
bool XmlWriter::addComment(const std::string &commentText)
{
    bool success = false;
    bool closeStartTag = false;
    State nextState = m_state;

    if (XmlValidator::validateCommentString(commentText))
    {
        switch (m_state)
        {
            case State_Empty:
            {
                nextState = State_DocumentStarted;
                success = true;
                break;
            }

            case State_ElementStarted:
            {
                // Close start tag of current element
                closeStartTag = true;
                nextState = State_InElement;
                success = true;
                break;
            }

            case State_DocumentStarted:
            case State_InElement:
            case State_DocumentEnded:
            {
                success = true;
                break;
            }

            default:
            {
                break;
            }
        }
    }

    if (success)
    {
        if (closeStartTag)
        {
            m_xmlString.append(">");
            m_currentElementInfo.contentEmpty = false;
            m_attributeNameList.clear();
        }

        m_xmlString.append("<!--").append(commentText).append("-->");
        m_state = nextState;
    }

    return success;
}

/**
 * Adds a new processing instruction in the XML document
 *
 * \param piTarget  Processing instruction target name
 * \param piValue   Processing instruction value
 *
 * \retval true     Success
 * \retval false    Error
 */
bool XmlWriter::addProcessingInstruction(const std::string &piTarget, const std::string &piValue)
{
    bool success = false;
    bool closeStartTag = false;
    State nextState = m_state;

    if (XmlValidator::validatePiTargetString(piTarget))
    {
        if (XmlValidator::validatePiValueString(piValue))
        {
            switch (m_state)
            {
                case State_Empty:
                {
                    nextState = State_DocumentStarted;
                    success = true;
                    break;
                }

                case State_DocumentStarted:
                case State_InElement:
                case State_DocumentEnded:
                {
                    success = true;
                    break;
                }

                case State_ElementStarted:
                {
                    // Close start tag of current element
                    closeStartTag = true;
                    nextState = State_InElement;
                    success = true;
                    break;
                }

                default:
                {
                    break;
                }
            }
        }
    }

    if (success)
    {
        if (closeStartTag)
        {
            m_xmlString.append(">");
            m_currentElementInfo.contentEmpty = false;
            m_attributeNameList.clear();
        }

        m_xmlString.append("<?").append(piTarget);

        if (!piValue.empty())
        {
            m_xmlString.append(" ").append(piValue);
        }

        m_xmlString.append("?>");
        m_state = State_ElementStarted;
    }

    return success;
}

/**
 * Starts a new element in the XML document
 *
 * \param elementName   Element name
 *
 * \retval true     Success
 * \retval false    Error
 */
bool XmlWriter::startElement(const std::string &elementName)
{
    bool success = false;
    bool closeStartTag = false;

    if (XmlValidator::validateNameString(elementName))
    {
        switch (m_state)
        {
            case State_DocumentStarted:
            {
                // No validation of root element name is required if document type is not set
                if (m_documentType.empty())
                {
                    success = true;
                }
                // Check if element name matches the document type
                else if (elementName == m_documentType)
                {
                    success = true;
                }
                else
                {
                    // Error, invalid root element name
                }

                break;
            }

            case State_Empty:
            case State_InElement:
            {
                success = true;
                break;
            }

            case State_ElementStarted:
            {
                // Close start tag of current element
                closeStartTag = true;
                success = true;
                break;
            }

            default:
            {
                break;
            }
        }
    }

    if (success)
    {
        if (closeStartTag)
        {
            m_xmlString.append(">");
            m_currentElementInfo.contentEmpty = false;
            m_attributeNameList.clear();
        }

        if (!m_currentElementInfo.name.empty())
        {
            // Add current element info to opened element list. If current element's name is empty
            // this can only mean that we have just started the root element, so there is noting to
            // add to the list yet.
            m_openedElementList.push_back(m_currentElementInfo);
        }

        // Open start tag
        m_xmlString.append("<").append(elementName);

        m_currentElementInfo = ElementInfo(elementName);
        m_state = State_ElementStarted;
        success = true;
    }

    return success;
}

/**
 * Add the attribute to the current element
 *
 * \param attribute     Attribute
 * \param quotationMark Quotation mark
 *
 * \retval true     Success
 * \retval false    Error
 */
bool XmlWriter::addAttribute(const Attribute &attribute, const Common::QuotationMark quotationMark)
{
    bool success = false;

    if (m_state == State_ElementStarted)
    {
        if (XmlValidator::validateNameString(attribute.name))
        {
            // Check if an attribute with the same name has already been added to the element
            bool attributeNameFound = false;
            
            for (size_t i = 0U; i < m_attributeNameList.size(); i++)
            {
                if (attribute.name == m_attributeNameList.at(i))
                {
                    // Error, an attribute with the same name is already in the list
                    attributeNameFound = true;
                    break;
                }
            }

            // Attribute names must be unique within an elements start tag
            if (!attributeNameFound)
            {
                const std::string escapedAttrValue = escapeAttValue(attribute.value, quotationMark);

                if ((escapedAttrValue.empty() && attribute.value.empty()) ||
                    (!escapedAttrValue.empty() && !attribute.value.empty()))
                {
                    // AttValue was successfully escaped
                    if (XmlValidator::validateAttValueString(escapedAttrValue))
                    {
                        char quotationMarkCharacter;

                        switch (quotationMark)
                        {
                            case Common::QuotationMark_Quote:
                            {
                                quotationMarkCharacter = '"';
                                success = true;
                                break;
                            }

                            case Common::QuotationMark_Apostrophe:
                            {
                                quotationMarkCharacter = '\'';
                                success = true;
                                break;
                            }

                            default:
                            {
                                // Error, invalid quotation mark
                                break;
                            }
                        }

                        if (success)
                        {
                            m_attributeNameList.append(attribute.name);
                            
                            m_xmlString.append(" ").append(attribute.name);
                            m_xmlString.append("=").append(1U, quotationMarkCharacter);
                            m_xmlString.append(escapedAttrValue).append(1U, quotationMarkCharacter);
                            success = true;
                        }
                    }
                }
            }
        }
    }

    return success;
}

/**
 * Add a text node
 *
 * \param textNode  UTF-8 encoded unescaped string
 *
 * \retval true     Success
 * \retval false    Error
 */
bool XmlWriter::addTextNode(const std::string &textNode)
{
    bool success = false;
    bool closeStartTag = false;
    
    // TODO: escape text node string

    if (XmlValidator::validateTextNodeString(textNode))
    {
        switch (m_state)
        {
            case State_InElement:
            {
                success = true;
                break;
            }

            case State_ElementStarted:
            {
                // Close start tag of current element
                closeStartTag = true;
                success = true;
                break;
            }

            default:
            {
                break;
            }
        }
    }

    if (success)
    {
        if (closeStartTag)
        {
            m_xmlString.append(">");
            m_currentElementInfo.contentEmpty = false;
            m_attributeNameList.clear();
        }

        m_xmlString.append(textNode);
        m_state = State_ElementStarted;
        success = true;
    }

    return success;
}

//-----------------------------------------------


bool XmlWriter::endElement()
{

}


//-----------------------------------------------






/**
 * Create an escaped AttValue string
 *
 * \param rawAttValue   Unescaped UTF-8 encoded AttValue string
 * \param quotationMark Quotation mark for the attribute
 *
 * \return Escaped AttValue string
 * \return Empty string on error
 */
std::string XmlWriter::escapeAttValue(const std::string &unescapedAttValue,
                                      const Common::QuotationMark quotationMark)
{
    std::string escapedString;

    if (!unescapedAttValue.empty())
    {
        escapedString.reserve(unescapedAttValue.size());  // Minimize allocations

        bool characterEscaped = false;
        bool success = false;
        uint32_t unicodeCharacter = 0U;
        size_t currentPosition = 0U;
        size_t nextPosition = 0U;

        do
        {
            Utf::Result result = Utf::unicodeCharacterFromUtf8(
                                     unescapedAttValue,
                                     currentPosition,
                                     &nextPosition,
                                     &unicodeCharacter);

            success = false;

            if (result == Utf::Result_Success)
            {
                // Check if character has to be escaped
                switch (unicodeCharacter)
                {
                    case (uint32_t)'<':
                    case (uint32_t)'&':
                    case (uint32_t)'"':
                    case (uint32_t)'\'':
                    {
                        if ((unicodeCharacter == (uint32_t)'"') &&
                            (quotationMark == Common::QuotationMark_Apostrophe))
                        {
                            // No need to escape a quote character if the quotation mark is an
                            // apostrophe
                        }
                        else if ((unicodeCharacter == (uint32_t)'\'') &&
                                 (quotationMark == Common::QuotationMark_Quote))
                        {
                            // No need to escape an apostrophe character if the quotation mark is a
                            // quote
                        }
                        else
                        {
                            // Escape all others
                            const std::string escapedSpecialCharacter =
                                Common::escapeSpecialCharacter(unicodeCharacter);

                            if (!escapedSpecialCharacter.empty())
                            {
                                if (!characterEscaped)
                                {
                                    // This is the first escaped character in the string. Copy the
                                    // raw string up to the escaped character
                                    const size_t len = currentPosition;

                                    if (len > 0U)
                                    {
                                        escapedString.append(unescapedAttValue.substr(0U, len));
                                    }

                                    characterEscaped = true;
                                }

                                escapedString.append(escapedSpecialCharacter);
                                success = true;
                            }
                        }

                        break;
                    }

                    default:
                    {
                        if (characterEscaped)
                        {
                            const size_t len = nextPosition - currentPosition;

                            if (len == 1U)
                            {
                                // Single byte unicode character
                                escapedString.append(1U, unescapedAttValue.at(currentPosition));
                            }
                            else
                            {
                                // Multibyte unicode character
                                escapedString.append(unescapedAttValue.substr(currentPosition, len));
                            }
                        }

                        success = true;
                        break;
                    }
                }

                if (success)
                {
                    currentPosition = nextPosition;
                }
            }
        }
        while (success && (nextPosition < unescapedAttValue.size()));

        if (success)
        {
            if (!characterEscaped)
            {
                // No character was escaped, so we can just copy the raw string
                escapedString = unescapedAttValue;
            }
        }
        else
        {
            // On error make sure that the escapedString is empty
            if (!escapedString.empty())
            {
                escapedString.clear();
            }
        }
    }

    return escapedString;
}
