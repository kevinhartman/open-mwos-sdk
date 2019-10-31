#pragma once

class TranslationUnit;

class ROFWriter {
public:
    ROFWriter();

    void Write(TranslationUnit* translation_unit);
};
