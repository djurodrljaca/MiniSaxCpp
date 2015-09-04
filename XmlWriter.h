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
#include "Attribute.h"

namespace MiniSaxCpp
{
/**
 * XML Writer class can be used to create a XML document
 */
class XmlWriter
{
public:
    XmlWriter();

    void clearMessage();
    std::string getMessage() const;

    bool setXmlDeclaration();
    void addComment(const std::string &comment);

    bool startElement(const std::string &elementName);
    bool addAttribute(const Attribute &attribute);
    void addTextNode(const std::string &textNode);
    bool endElement();
    

private:
    enum State
    {
        State_Empty,
        State_DocumentStarted,
        State_ElementStarted,
        State_InElement,
        State_DocumentEnded
    };
    
    static std::string createAttributeString(const Attribute &attribute);
    static std::string createStartOfElementString(const std::string &elementName,
            const AttributeList &attributeList = AttributeList());

    State m_state;
    bool m_xmlDeclarationSet;
    std::list<std::string> m_elementNameList;
    // TODO: attribute name list for the currently started element?
    std::string m_message;
    
    // TODO: pretty printing?
};
}
