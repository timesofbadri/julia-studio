#ifndef QMLEXPRESSIONUNDERCURSOR_H
#define QMLEXPRESSIONUNDERCURSOR_H

#include <QStack>
#include <QTextBlock>
#include <QTextCursor>

#include "qmldocument.h"
#include "qmljsastfwd_p.h"
#include "qmlsymbol.h"

namespace QmlJS {
    class Engine;
    class NodePool;
}

namespace QmlEditor {
namespace Internal {

class QmlExpressionUnderCursor
{
public:
    QmlExpressionUnderCursor();
    virtual ~QmlExpressionUnderCursor();

    void operator()(const QTextCursor &cursor, const QmlDocument::Ptr &doc);

    QStack<QmlSymbol *> expressionScopes() const
    { return _expressionScopes; }

    QmlJS::AST::Node *expressionNode() const
    { return _expressionNode; }

    int expressionOffset() const
    { return _expressionOffset; }

    int expressionLength() const
    { return _expressionLength; }

private:
    void parseExpression(const QTextBlock &block);

    QmlJS::AST::ExpressionNode *tryExpression(const QString &text);
    QmlJS::AST::Statement *tryStatement(const QString &text);
    QmlJS::AST::UiObjectMember *tryBinding(const QString &text);

private:
    QStack<QmlSymbol *> _expressionScopes;
    QmlJS::AST::Node *_expressionNode;
    int _expressionOffset;
    int _expressionLength;
    quint32 _pos;
    QmlJS::Engine *_engine;
    QmlJS::NodePool *_nodePool;
};

} // namespace Internal
} // namespace QmlEditor

#endif // QMLEXPRESSIONUNDERCURSOR_H
