

#include "config.h"
#include "HTMLTextAreaElement.h"

#include "BeforeTextInsertedEvent.h"
#include "Chrome.h"
#include "ChromeClient.h"
#include "CSSValueKeywords.h"
#include "Document.h"
#include "Event.h"
#include "EventNames.h"
#include "ExceptionCode.h"
#include "FocusController.h"
#include "FormDataList.h"
#include "Frame.h"
#include "HTMLNames.h"
#include "InputElement.h"
#include "MappedAttribute.h"
#include "Page.h"
#include "RenderStyle.h"
#include "RenderTextControlMultiLine.h"
#include "ScriptEventListener.h"
#include "Text.h"
#include "TextIterator.h"
#include "VisibleSelection.h"
#include <wtf/StdLibExtras.h>

#ifdef ANDROID_ACCEPT_CHANGES_TO_FOCUSED_TEXTFIELDS
#include "WebViewCore.h"
#endif

namespace WebCore {

using namespace HTMLNames;

static const int defaultRows = 2;
static const int defaultCols = 20;

static inline void notifyFormStateChanged(const HTMLTextAreaElement* element)
{
    Frame* frame = element->document()->frame();
    if (!frame)
        return;
    frame->page()->chrome()->client()->formStateDidChange(element);
}

HTMLTextAreaElement::HTMLTextAreaElement(const QualifiedName& tagName, Document* document, HTMLFormElement* form)
    : HTMLTextFormControlElement(tagName, document, form)
    , m_rows(defaultRows)
    , m_cols(defaultCols)
    , m_wrap(SoftWrap)
    , m_cachedSelectionStart(-1)
    , m_cachedSelectionEnd(-1)
    , m_isDirty(false)
{
    ASSERT(hasTagName(textareaTag));
    setFormControlValueMatchesRenderer(true);
    notifyFormStateChanged(this);
}

const AtomicString& HTMLTextAreaElement::formControlType() const
{
    DEFINE_STATIC_LOCAL(const AtomicString, textarea, ("textarea"));
    return textarea;
}

bool HTMLTextAreaElement::saveFormControlState(String& result) const
{
    result = value();
    return true;
}

void HTMLTextAreaElement::restoreFormControlState(const String& state)
{
    setValue(state);
}

void HTMLTextAreaElement::childrenChanged(bool changedByParser, Node* beforeChange, Node* afterChange, int childCountDelta)
{
    setNonDirtyValue(defaultValue());
    HTMLElement::childrenChanged(changedByParser, beforeChange, afterChange, childCountDelta);
}
    
void HTMLTextAreaElement::parseMappedAttribute(MappedAttribute* attr)
{
    if (attr->name() == rowsAttr) {
        int rows = attr->value().toInt();
        if (rows <= 0)
            rows = defaultRows;
        if (m_rows != rows) {
            m_rows = rows;
            if (renderer())
                renderer()->setNeedsLayoutAndPrefWidthsRecalc();
        }
    } else if (attr->name() == colsAttr) {
        int cols = attr->value().toInt();
        if (cols <= 0)
            cols = defaultCols;
        if (m_cols != cols) {
            m_cols = cols;
            if (renderer())
                renderer()->setNeedsLayoutAndPrefWidthsRecalc();
        }
    } else if (attr->name() == wrapAttr) {
        // The virtual/physical values were a Netscape extension of HTML 3.0, now deprecated.
        // The soft/hard /off values are a recommendation for HTML 4 extension by IE and NS 4.
        WrapMethod wrap;
        if (equalIgnoringCase(attr->value(), "physical") || equalIgnoringCase(attr->value(), "hard") || equalIgnoringCase(attr->value(), "on"))
            wrap = HardWrap;
        else if (equalIgnoringCase(attr->value(), "off"))
            wrap = NoWrap;
        else
            wrap = SoftWrap;
        if (wrap != m_wrap) {
            m_wrap = wrap;

            if (shouldWrapText()) {
                addCSSProperty(attr, CSSPropertyWhiteSpace, CSSValuePreWrap);
                addCSSProperty(attr, CSSPropertyWordWrap, CSSValueBreakWord);
            } else {
                addCSSProperty(attr, CSSPropertyWhiteSpace, CSSValuePre);
                addCSSProperty(attr, CSSPropertyWordWrap, CSSValueNormal);
            }

            if (renderer())
                renderer()->setNeedsLayoutAndPrefWidthsRecalc();
        }
    } else if (attr->name() == accesskeyAttr) {
        // ignore for the moment
    } else if (attr->name() == alignAttr) {
        // Don't map 'align' attribute.  This matches what Firefox, Opera and IE do.
        // See http://bugs.webkit.org/show_bug.cgi?id=7075
    } else if (attr->name() == maxlengthAttr)
        setNeedsValidityCheck();
    else
        HTMLTextFormControlElement::parseMappedAttribute(attr);
}

RenderObject* HTMLTextAreaElement::createRenderer(RenderArena* arena, RenderStyle*)
{
    return new (arena) RenderTextControlMultiLine(this, placeholderShouldBeVisible());
}

bool HTMLTextAreaElement::appendFormData(FormDataList& encoding, bool)
{
    if (name().isEmpty())
        return false;

    // FIXME: It's not acceptable to ignore the HardWrap setting when there is no renderer.
    // While we have no evidence this has ever been a practical problem, it would be best to fix it some day.
    RenderTextControl* control = toRenderTextControl(renderer());
    const String& text = (m_wrap == HardWrap && control) ? control->textWithHardLineBreaks() : value();
    encoding.appendData(name(), text);
    return true;
}

void HTMLTextAreaElement::reset()
{
    setNonDirtyValue(defaultValue());
}

bool HTMLTextAreaElement::isKeyboardFocusable(KeyboardEvent*) const
{
    // If a given text area can be focused at all, then it will always be keyboard focusable.
    return isFocusable();
}

bool HTMLTextAreaElement::isMouseFocusable() const
{
    return isFocusable();
}

void HTMLTextAreaElement::updateFocusAppearance(bool restorePreviousSelection)
{
    ASSERT(renderer());
    ASSERT(!document()->childNeedsAndNotInStyleRecalc());

    if (!restorePreviousSelection || m_cachedSelectionStart < 0) {
#if ENABLE(ON_FIRST_TEXTAREA_FOCUS_SELECT_ALL)
        // Devices with trackballs or d-pads may focus on a textarea in route
        // to another focusable node. By selecting all text, the next movement
        // can more readily be interpreted as moving to the next node.
        select();
#else
        // If this is the first focus, set a caret at the beginning of the text.  
        // This matches some browsers' behavior; see bug 11746 Comment #15.
        // http://bugs.webkit.org/show_bug.cgi?id=11746#c15
        setSelectionRange(0, 0);
#endif
    } else {
        // Restore the cached selection.  This matches other browsers' behavior.
        setSelectionRange(m_cachedSelectionStart, m_cachedSelectionEnd);
    }

    if (document()->frame())
        document()->frame()->revealSelection();
}

void HTMLTextAreaElement::defaultEventHandler(Event* event)
{
    if (renderer() && (event->isMouseEvent() || event->isDragEvent() || event->isWheelEvent() || event->type() == eventNames().blurEvent))
        toRenderTextControlMultiLine(renderer())->forwardEvent(event);
    else if (renderer() && event->isBeforeTextInsertedEvent())
        handleBeforeTextInsertedEvent(static_cast<BeforeTextInsertedEvent*>(event));

    HTMLFormControlElementWithState::defaultEventHandler(event);
}

void HTMLTextAreaElement::handleBeforeTextInsertedEvent(BeforeTextInsertedEvent* event) const
{
    ASSERT(event);
    ASSERT(renderer());
    int signedMaxLength = maxLength();
    if (signedMaxLength < 0)
        return;
    unsigned unsignedMaxLength = static_cast<unsigned>(signedMaxLength);

    unsigned currentLength = toRenderTextControl(renderer())->text().numGraphemeClusters();
    unsigned selectionLength = plainText(document()->frame()->selection()->selection().toNormalizedRange().get()).numGraphemeClusters();
    ASSERT(currentLength >= selectionLength);
    unsigned baseLength = currentLength - selectionLength;
    unsigned appendableLength = unsignedMaxLength > baseLength ? unsignedMaxLength - baseLength : 0;
    event->setText(sanitizeUserInputValue(event->text(), appendableLength));
}

String HTMLTextAreaElement::sanitizeUserInputValue(const String& proposedValue, unsigned maxLength)
{
    return proposedValue.left(proposedValue.numCharactersInGraphemeClusters(maxLength));
}

void HTMLTextAreaElement::rendererWillBeDestroyed()
{
    updateValue();
}

void HTMLTextAreaElement::updateValue() const
{
    if (formControlValueMatchesRenderer())
        return;

    ASSERT(renderer());
    m_value = toRenderTextControl(renderer())->text();
    const_cast<HTMLTextAreaElement*>(this)->setFormControlValueMatchesRenderer(true);
    notifyFormStateChanged(this);
    m_isDirty = true;
}

String HTMLTextAreaElement::value() const
{
    updateValue();
    return m_value;
}

void HTMLTextAreaElement::setValue(const String& value)
{
    setNonDirtyValue(value);
    m_isDirty = true;
}

void HTMLTextAreaElement::setNonDirtyValue(const String& value)
{
    // Code elsewhere normalizes line endings added by the user via the keyboard or pasting.
    // We normalize line endings coming from JavaScript here.
    String normalizedValue = value.isNull() ? "" : value;
    normalizedValue.replace("\r\n", "\n");
    normalizedValue.replace('\r', '\n');

    // Return early because we don't want to move the caret or trigger other side effects
    // when the value isn't changing. This matches Firefox behavior, at least.
    if (normalizedValue == this->value())
        return;

    m_value = normalizedValue;
    m_isDirty = false;
    setFormControlValueMatchesRenderer(true);
    updatePlaceholderVisibility(false);
    if (inDocument())
        document()->updateStyleIfNeeded();
    if (renderer())
        renderer()->updateFromElement();

    // Set the caret to the end of the text value.
    if (document()->focusedNode() == this) {
#ifdef ANDROID_ACCEPT_CHANGES_TO_FOCUSED_TEXTFIELDS
        // Make sure our UI side textfield changes to match the RenderTextControl
        android::WebViewCore::getWebViewCore(document()->view())->updateTextfield(this, false, value);
#endif
        unsigned endOfString = m_value.length();
        setSelectionRange(endOfString, endOfString);
    }

    setNeedsValidityCheck();
    notifyFormStateChanged(this);
}

String HTMLTextAreaElement::defaultValue() const
{
    String value = "";

    // Since there may be comments, ignore nodes other than text nodes.
    for (Node* n = firstChild(); n; n = n->nextSibling()) {
        if (n->isTextNode())
            value += static_cast<Text*>(n)->data();
    }

    UChar firstCharacter = value[0];
    if (firstCharacter == '\r' && value[1] == '\n')
        value.remove(0, 2);
    else if (firstCharacter == '\r' || firstCharacter == '\n')
        value.remove(0, 1);

    return value;
}

void HTMLTextAreaElement::setDefaultValue(const String& defaultValue)
{
    // To preserve comments, remove only the text nodes, then add a single text node.

    Vector<RefPtr<Node> > textNodes;
    for (Node* n = firstChild(); n; n = n->nextSibling()) {
        if (n->isTextNode())
            textNodes.append(n);
    }
    ExceptionCode ec;
    size_t size = textNodes.size();
    for (size_t i = 0; i < size; ++i)
        removeChild(textNodes[i].get(), ec);

    // Normalize line endings.
    // Add an extra line break if the string starts with one, since
    // the code to read default values from the DOM strips the leading one.
    String value = defaultValue;
    value.replace("\r\n", "\n");
    value.replace('\r', '\n');
    if (value[0] == '\n')
        value = "\n" + value;

    insertBefore(document()->createTextNode(value), firstChild(), ec);

    setNonDirtyValue(value);
}

int HTMLTextAreaElement::maxLength() const
{
    bool ok;
    int value = getAttribute(maxlengthAttr).string().toInt(&ok);
    return ok && value >= 0 ? value : -1;
}

void HTMLTextAreaElement::setMaxLength(int newValue, ExceptionCode& ec)
{
    if (newValue < 0)
        ec = INDEX_SIZE_ERR;
    else
        setAttribute(maxlengthAttr, String::number(newValue));
}

bool HTMLTextAreaElement::tooLong() const
{
    // Return false for the default value even if it is longer than maxLength.
    if (!m_isDirty)
        return false;

    int max = maxLength();
    if (max < 0)
        return false;
    return value().numGraphemeClusters() > static_cast<unsigned>(max);
}

void HTMLTextAreaElement::accessKeyAction(bool)
{
    focus();
}

const AtomicString& HTMLTextAreaElement::accessKey() const
{
    return getAttribute(accesskeyAttr);
}

void HTMLTextAreaElement::setAccessKey(const String& value)
{
    setAttribute(accesskeyAttr, value);
}

void HTMLTextAreaElement::setCols(int cols)
{
    setAttribute(colsAttr, String::number(cols));
}

void HTMLTextAreaElement::setRows(int rows)
{
    setAttribute(rowsAttr, String::number(rows));
}

bool HTMLTextAreaElement::shouldUseInputMethod() const
{
    return true;
}

} // namespace
