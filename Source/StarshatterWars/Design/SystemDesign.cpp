/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (C) 2025-2026. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         SystemDesign.cpp
	AUTHOR:       Carlos Bott

	ORIGINAL AUTHOR AND STUDIO
	==========================
	John DiCamillo / Destroyer Studios LLC

	OVERVIEW
	========
	Generic ship System Design class
*/

#include "SystemDesign.h"
#include "CoreMinimal.h" // UE_LOG, ANSI_TO_TCHAR
#include "SimComponent.h"

#include "Game.h"
// Bitmap removed: render assets are Unreal UTexture2D*
#include "DataLoader.h"
#include "ParseUtil.h"
#include "List.h"

// +--------------------------------------------------------------------+

List<SystemDesign> SystemDesign::catalog;

#define GET_DEF_TEXT(p,d,x) if(p->name()->value()==(#x))GetDefText(d->x,p,filename)
#define GET_DEF_NUM(p,d,x)  if(p->name()->value()==(#x))GetDefNumber(d->x,p,filename)

// +--------------------------------------------------------------------+

SystemDesign::SystemDesign()
{
}

SystemDesign::~SystemDesign()
{
	components.destroy();
}

// +--------------------------------------------------------------------+

void
SystemDesign::Initialize(const char* filename)
{
	UE_LOG(LogTemp, Log, TEXT("Loading System Designs '%s'"), ANSI_TO_TCHAR(filename));

	// Load Design File:
	DataLoader* loader = DataLoader::GetLoader();
	BYTE* block;

	int blocklen = loader->LoadBuffer(filename, block, true);
	Parser parser(new BlockReader((const char*)block, blocklen));
	Term* term = parser.ParseTerm();

	if (!term) {
		UE_LOG(LogTemp, Error, TEXT("ERROR: could notxparse '%s'"), ANSI_TO_TCHAR(filename));
		// Original code called exit(-3); keep fatal behavior but in Unreal-friendly form:
		checkf(false, TEXT("SystemDesign::Initialize parse failure: %s"), ANSI_TO_TCHAR(filename));
		return;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "SYSTEM") {
			UE_LOG(LogTemp, Error, TEXT("ERROR: invalid system design file '%s'"), ANSI_TO_TCHAR(filename));
			// Original code called exit(-4); keep fatal behavior but in Unreal-friendly form:
			checkf(false, TEXT("SystemDesign::Initialize invalid file type: %s"), ANSI_TO_TCHAR(filename));
			return;
		}
	}

	do {
		delete term;

		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				if (def->name()->value() == "system") {

					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Warning, TEXT("WARNING: system structure missing in '%s'"), ANSI_TO_TCHAR(filename));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						SystemDesign* design = new SystemDesign;

						for (int i = 0; i < val->elements()->size(); i++) {
							TermDef* pdef = val->elements()->at(i)->isDef();
							if (pdef) {
								GET_DEF_TEXT(pdef, design, name);

							else if (pdef->name()->value() == ("component")) {
								if (!pdef->term() || !pdef->term()->isStruct()) {
									UE_LOG(LogTemp, Warning,
										TEXT("WARNING: component structure missing in system '%s' in '%s'"),
										ANSI_TO_TCHAR(design->name.data()),
										ANSI_TO_TCHAR(filename));
								}
								else {
									TermStruct* val2 = pdef->term()->isStruct();
									ComponentDesign* comp_design = new ComponentDesign;

									for (int j = 0; j < val2->elements()->size(); j++) {
										TermDef* pdef2 = val2->elements()->at(j)->isDef();
										if (pdef2) {
											GET_DEF_TEXT(pdef2, comp_design, name);
										else GET_DEF_TEXT(pdef2, comp_design, abrv);
							else GET_DEF_NUM(pdef2, comp_design, repair_time);
				else GET_DEF_NUM(pdef2, comp_design, replace_time);
			else GET_DEF_NUM(pdef2, comp_design, spares);
		else GET_DEF_NUM(pdef2, comp_design, affects);

else {
	UE_LOG(LogTemp, Warning,
		TEXT("WARNING: parameter '%s' ignored in '%s'"),
		ANSI_TO_TCHAR(pdef2->name()->value().data()),
		ANSI_TO_TCHAR(filename));
}
										}
									}

									design->components.append(comp_design);
								}
							}

							else {
								UE_LOG(LogTemp, Warning,
									TEXT("WARNING: parameter '%s' ignored in '%s'"),
									ANSI_TO_TCHAR(pdef->name()->value().data()),
									ANSI_TO_TCHAR(filename));
							}
							}
							else {
								UE_LOG(LogTemp, Warning, TEXT("WARNING: term ignored in '%s'"), ANSI_TO_TCHAR(filename));
								val->elements()->at(i)->print();
							}
						}

						catalog.append(design);
					}
				}

				else {
					UE_LOG(LogTemp, Warning,
						TEXT("WARNING: unknown definition '%s' in '%s'"),
						ANSI_TO_TCHAR(def->name()->value().data()),
						ANSI_TO_TCHAR(filename));
				}
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("WARNING: term ignored in '%s'"), ANSI_TO_TCHAR(filename));
				term->print();
			}
		}
	} while (term);

	loader->ReleaseBuffer(block);
}

// +--------------------------------------------------------------------+

void
SystemDesign::Close()
{
	catalog.destroy();
}

// +--------------------------------------------------------------------+

SystemDesign*
SystemDesign::Find(const char* name)
{
	SystemDesign test;
	test.name = name;
	return catalog.find(&test);
}
