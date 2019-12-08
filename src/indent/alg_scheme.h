#pragma once

#include <QString>
#include <QTextBlock>

#include "indenter.h"


namespace Qutepart {

class IndentAlgScheme: public IndentAlgImpl {
public:
    QString computeSmartIndent(QTextBlock block) const override;
};

};  // namespace Qutepart
