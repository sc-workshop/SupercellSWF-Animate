#include "SharedPublisher/ResourcePalette.h"

#include "ApplicationFCMPublicIDs.h"

#include "MediaInfo/IBitmapInfo.h"
#include "MediaInfo/ISoundInfo.h"
#include "ILibraryItem.h"

#include "FrameElement/IShape.h"

#include "StrokeStyle/IDashedStrokeStyle.h"
#include "StrokeStyle/IDottedStrokeStyle.h"
#include "StrokeStyle/IHatchedStrokeStyle.h"
#include "StrokeStyle/IRaggedStrokeStyle.h"
#include "StrokeStyle/ISolidStrokeStyle.h"
#include "StrokeStyle/IStippleStrokeStyle.h"
#include "StrokeStyle/IStrokeWidth.h"

#include "FrameElement/IParagraph.h"
#include "FrameElement/ITextRun.h"
#include "FrameElement/ITextBehaviour.h"

#include "Service/Shape/IRegionGeneratorService.h"
#include "Service/Shape/IFilledRegion.h"
#include "Service/Shape/IStrokeGroup.h"
#include "Service/Shape/IEdge.h"
#include "Service/Shape/IShapeService.h"

#include "Utils/ILinearColorGradient.h"
#include "Utils/IRadialColorGradient.h"

#include "Utils.h"
#include "SharedPublisher/TimelineBuilder.h"

namespace SupercellSWF {
    FCM::Result ResourcePalette::AddSymbol(
        FCM::U_Int32 resourceId,
        FCM::StringRep16 pName,
        Exporter::Service::PITimelineBuilder pTimelineBuilder)
    {
        FCM::Result res;
        TimelineWriter* pTimelineWriter;

        Log(GetCallback(), "[EndSymbol] ResId: %d\n", resourceId);

        m_resourceList.push_back(resourceId);

        if (pName != NULL)
        {
            m_resourceNames.push_back(Utils::ToString(pName, GetCallback()));
        }

        TimelineBuilder* pTimeline = static_cast<TimelineBuilder*>(pTimelineBuilder);

        res = pTimeline->Build(resourceId, pName, &pTimelineWriter);

        return res;
    }


    FCM::Result ResourcePalette::AddShape(
        FCM::U_Int32 resourceId,
        DOM::FrameElement::PIShape pShape)
    {
        FCM::Result res;
        FCM::Boolean hasFancy;
        FCM::AutoPtr<DOM::FrameElement::IShape> pNewShape;

        Log(GetCallback(), "[DefineShape] ResId: %d\n", resourceId);

        m_resourceList.push_back(resourceId);
        m_pOutputWriter->StartDefineShape();

        if (pShape)
        {
            ExportFill(pShape);

            res = HasFancyStrokes(pShape, hasFancy);
            if (hasFancy)
            {
                res = ConvertStrokeToFill(pShape, pNewShape.m_Ptr);
                ASSERT(FCM_SUCCESS_CODE(res));

                ExportFill(pNewShape);
            }
            else
            {

                ExportStroke(pShape);
            }
        }

        m_pOutputWriter->EndDefineShape(resourceId);

        return FCM_SUCCESS;
    }


    FCM::Result ResourcePalette::AddSound(FCM::U_Int32 resourceId, DOM::LibraryItem::PIMediaItem pMediaItem)
    {
        FCM::Result res;
        DOM::AutoPtr<DOM::ILibraryItem> pLibItem;
        FCM::AutoPtr<FCM::IFCMUnknown> pUnknown;
        FCM::StringRep16 pName;
        std::string libName;

        Log(GetCallback(), "[DefineSound] ResId: %d\n", resourceId);

        m_resourceList.push_back(resourceId);

        // Store the resource name
        pLibItem = pMediaItem;

        res = pLibItem->GetName(&pName);
        ASSERT(FCM_SUCCESS_CODE(res));
        libName = Utils::ToString(pName, GetCallback());
        m_resourceNames.push_back(libName);

        res = pMediaItem->GetMediaInfo(pUnknown.m_Ptr);
        ASSERT(FCM_SUCCESS_CODE(res));

        AutoPtr<DOM::MediaInfo::ISoundInfo> pSoundInfo = pUnknown;
        ASSERT(pSoundInfo);

        m_pOutputWriter->DefineSound(resourceId, libName, pMediaItem);

        // Free the name
        FCM::AutoPtr<FCM::IFCMUnknown> pUnkCalloc;
        res = GetCallback()->GetService(SRVCID_Core_Memory, pUnkCalloc.m_Ptr);
        AutoPtr<FCM::IFCMCalloc> callocService = pUnkCalloc;

        callocService->Free((FCM::PVoid)pName);

        return FCM_SUCCESS;
    }


    FCM::Result ResourcePalette::AddBitmap(FCM::U_Int32 resourceId, DOM::LibraryItem::PIMediaItem pMediaItem)
    {
        DOM::AutoPtr<DOM::ILibraryItem> pLibItem;
        FCM::Result res;
        FCM::StringRep16 pName;

        Log(GetCallback(), "[DefineBitmap] ResId: %d\n", resourceId);

        m_resourceList.push_back(resourceId);

        pLibItem = pMediaItem;

        // Store the resource name
        res = pLibItem->GetName(&pName);
        ASSERT(FCM_SUCCESS_CODE(res));
        std::string libItemName = Utils::ToString(pName, GetCallback());
        m_resourceNames.push_back(libItemName);

        AutoPtr<FCM::IFCMUnknown> medInfo;
        pMediaItem->GetMediaInfo(medInfo.m_Ptr);

        AutoPtr<DOM::MediaInfo::IBitmapInfo> bitsInfo = medInfo;
        ASSERT(bitsInfo);

        // Get image height
        FCM::S_Int32 height;
        res = bitsInfo->GetHeight(height);
        ASSERT(FCM_SUCCESS_CODE(res));

        // Get image width
        FCM::S_Int32 width;
        res = bitsInfo->GetWidth(width);
        ASSERT(FCM_SUCCESS_CODE(res));

        // Dump the definition of a bitmap
        res = m_pOutputWriter->DefineBitmap(resourceId, height, width, libItemName, pMediaItem);

        // Free the name
        FCM::AutoPtr<FCM::IFCMUnknown> pUnkCalloc;
        res = GetCallback()->GetService(SRVCID_Core_Memory, pUnkCalloc.m_Ptr);
        AutoPtr<FCM::IFCMCalloc> callocService = pUnkCalloc;

        callocService->Free((FCM::PVoid)pName);

        return res;
    }


    FCM::Result ResourcePalette::AddClassicText(FCM::U_Int32 resourceId, DOM::FrameElement::PIClassicText pClassicText)
    {
        DOM::AutoPtr<DOM::FrameElement::IClassicText> pTextItem;
        FCMListPtr pParagraphsList;
        FCM::StringRep16 textDisplay;
        FCM::U_Int32 count = 0;
        FCM::U_Int16 fontSize;
        std::string fName;
        std::string displayText;
        DOM::Utils::COLOR fontColor;
        FCM::Result res;

        Log(GetCallback(), "[DefineClassicText] ResId: %d\n", resourceId);

        m_resourceList.push_back(resourceId);

        pTextItem = pClassicText;
        AutoPtr<DOM::FrameElement::ITextBehaviour> textBehaviour;
        pTextItem->GetTextBehaviour(textBehaviour.m_Ptr);
        AutoPtr<DOM::FrameElement::IDynamicTextBehaviour> dynamicTextBehaviour = textBehaviour.m_Ptr;

        if (dynamicTextBehaviour)
        {
            pTextItem->GetParagraphs(pParagraphsList.m_Ptr);
            res = pParagraphsList->Count(count);
            ASSERT(FCM_SUCCESS_CODE(res));

            res = pTextItem->GetText(&textDisplay);
            ASSERT(FCM_SUCCESS_CODE(res));
            displayText = Utils::ToString(textDisplay, GetCallback());

            // Free the textDisplay
            FCM::AutoPtr<FCM::IFCMUnknown> pUnkCalloc;
            res = GetCallback()->GetService(SRVCID_Core_Memory, pUnkCalloc.m_Ptr);
            AutoPtr<FCM::IFCMCalloc> callocService = pUnkCalloc;

            callocService->Free((FCM::PVoid)textDisplay);
        }

        for (FCM::U_Int32 pIndex = 0; pIndex < count; pIndex++)
        {
            AutoPtr<DOM::FrameElement::IParagraph> pParagraph = pParagraphsList[pIndex];

            if (pParagraph)
            {
                FCMListPtr pTextRunList;
                pParagraph->GetTextRuns(pTextRunList.m_Ptr);

                FCM::U_Int32 trCount;
                pTextRunList->Count(trCount);

                for (FCM::U_Int32 trIndex = 0; trIndex < trCount; trIndex++)
                {
                    AutoPtr<DOM::FrameElement::ITextRun> pTextRun = pTextRunList[trIndex];
                    AutoPtr<DOM::FrameElement::ITextStyle> trStyle;

                    pTextRun->GetTextStyle(trStyle.m_Ptr);

                    res = trStyle->GetFontSize(fontSize);
                    ASSERT(FCM_SUCCESS_CODE(res));

                    res = trStyle->GetFontColor(fontColor);
                    ASSERT(FCM_SUCCESS_CODE(res));

                    // Form font info in required format
                    GetFontInfo(trStyle, fName, fontSize);
                }
            }
        }

        //Define Text Element
        res = m_pOutputWriter->DefineText(resourceId, fName, fontColor, displayText, pTextItem);

        return FCM_SUCCESS;
    }



    FCM::Result ResourcePalette::HasResource(FCM::U_Int32 resourceId, FCM::Boolean& hasResource)
    {
        hasResource = false;

        for (std::vector<FCM::U_Int32>::iterator listIter = m_resourceList.begin();
            listIter != m_resourceList.end(); listIter++)
        {
            if (*listIter == resourceId)
            {
                hasResource = true;
                break;
            }
        }

        //Log(("[HasResource] ResId: %d HasResource: %d\n", resourceId, hasResource));

        return FCM_SUCCESS;
    }


    ResourcePalette::ResourcePalette()
    {
        m_pOutputWriter = NULL;
    }


    ResourcePalette::~ResourcePalette()
    {
    }


    void ResourcePalette::Init(OutputWriter* pOutputWriter)
    {
        m_pOutputWriter = pOutputWriter;
    }

    void ResourcePalette::Clear()
    {
        m_resourceList.clear();
    }

    FCM::Result ResourcePalette::HasResource(
        const std::string& name,
        FCM::Boolean& hasResource)
    {
        hasResource = false;
        for (FCM::U_Int32 index = 0; index < m_resourceNames.size(); index++)
        {
            if (m_resourceNames[index] == name)
            {
                hasResource = true;
                break;
            }
        }

        return FCM_SUCCESS;
    }


    FCM::Result ResourcePalette::ExportFill(DOM::FrameElement::PIShape pIShape)
    {
        FCM::Result res;
        FCM::FCMListPtr pFilledRegionList;
        FCM::AutoPtr<FCM::IFCMUnknown> pUnkSRVReg;
        FCM::U_Int32 regionCount;

        GetCallback()->GetService(DOM::FLA_REGION_GENERATOR_SERVICE, pUnkSRVReg.m_Ptr);
        AutoPtr<DOM::Service::Shape::IRegionGeneratorService> pIRegionGeneratorService(pUnkSRVReg);
        ASSERT(pIRegionGeneratorService);

        res = pIRegionGeneratorService->GetFilledRegions(pIShape, pFilledRegionList.m_Ptr);
        ASSERT(FCM_SUCCESS_CODE(res));

        pFilledRegionList->Count(regionCount);

        for (FCM::U_Int32 j = 0; j < regionCount; j++)
        {
            FCM::AutoPtr<DOM::Service::Shape::IFilledRegion> pFilledRegion = pFilledRegionList[j];
            FCM::AutoPtr<DOM::Service::Shape::IPath> pPath;

            m_pOutputWriter->StartDefineFill();

            // Fill Style
            FCM::AutoPtr<DOM::IFCMUnknown> fillStyle;

            res = pFilledRegion->GetFillStyle(fillStyle.m_Ptr);
            ASSERT(FCM_SUCCESS_CODE(res));

            ExportFillStyle(fillStyle);

            // Boundary
            res = pFilledRegion->GetBoundary(pPath.m_Ptr);
            ASSERT(FCM_SUCCESS_CODE(res));

            res = ExportFillBoundary(pPath);
            ASSERT(FCM_SUCCESS_CODE(res));

            // Hole List
            FCMListPtr pHoleList;
            FCM::U_Int32 holeCount;

            res = pFilledRegion->GetHoles(pHoleList.m_Ptr);
            ASSERT(FCM_SUCCESS_CODE(res));

            res = pHoleList->Count(holeCount);
            ASSERT(FCM_SUCCESS_CODE(res));

            for (FCM::U_Int32 k = 0; k < holeCount; k++)
            {
                FCM::FCMListPtr pEdgeList;
                FCM::AutoPtr<DOM::Service::Shape::IPath> pPath = pHoleList[k];

                res = ExportHole(pPath);
            }

            m_pOutputWriter->EndDefineFill();
        }

        return res;
    }


    FCM::Result ResourcePalette::ExportFillBoundary(DOM::Service::Shape::PIPath pPath)
    {
        FCM::Result res;

        m_pOutputWriter->StartDefineBoundary();

        res = ExportPath(pPath);
        ASSERT(FCM_SUCCESS_CODE(res));

        m_pOutputWriter->EndDefineBoundary();

        return res;
    }


    FCM::Result ResourcePalette::ExportHole(DOM::Service::Shape::PIPath pPath)
    {
        FCM::Result res;

        m_pOutputWriter->StartDefineHole();

        res = ExportPath(pPath);
        ASSERT(FCM_SUCCESS_CODE(res));

        m_pOutputWriter->EndDefineHole();

        return res;
    }


    FCM::Result ResourcePalette::ExportPath(DOM::Service::Shape::PIPath pPath)
    {
        FCM::Result res;
        FCM::U_Int32 edgeCount;
        FCM::FCMListPtr pEdgeList;

        res = pPath->GetEdges(pEdgeList.m_Ptr);
        ASSERT(FCM_SUCCESS_CODE(res));

        res = pEdgeList->Count(edgeCount);
        ASSERT(FCM_SUCCESS_CODE(res));

        for (FCM::U_Int32 l = 0; l < edgeCount; l++)
        {
            DOM::Utils::SEGMENT segment;

            segment.structSize = sizeof(DOM::Utils::SEGMENT);

            FCM::AutoPtr<DOM::Service::Shape::IEdge> pEdge = pEdgeList[l];

            res = pEdge->GetSegment(segment);

            m_pOutputWriter->SetSegment(segment);
        }

        return res;
    }

    FCM::Result ResourcePalette::ExportFillStyle(FCM::PIFCMUnknown pFillStyle)
    {
        FCM::Result res = FCM_SUCCESS;

        AutoPtr<DOM::FillStyle::ISolidFillStyle> pSolidFillStyle;
        AutoPtr<DOM::FillStyle::IGradientFillStyle> pGradientFillStyle;
        AutoPtr<DOM::FillStyle::IBitmapFillStyle> pBitmapFillStyle;

        // Check for solid fill color
        pSolidFillStyle = pFillStyle;
        if (pSolidFillStyle)
        {
            res = ExportSolidFillStyle(pSolidFillStyle);
            ASSERT(FCM_SUCCESS_CODE(res));
        }

        // Check for Gradient Fill
        pGradientFillStyle = pFillStyle;
        AutoPtr<FCM::IFCMUnknown> pGrad;

        if (pGradientFillStyle)
        {
            pGradientFillStyle->GetColorGradient(pGrad.m_Ptr);

            if (AutoPtr<DOM::Utils::IRadialColorGradient>(pGrad))
            {
                res = ExportRadialGradientFillStyle(pGradientFillStyle);
                ASSERT(FCM_SUCCESS_CODE(res));
            }
            else if (AutoPtr<DOM::Utils::ILinearColorGradient>(pGrad))
            {
                res = ExportLinearGradientFillStyle(pGradientFillStyle);
                ASSERT(FCM_SUCCESS_CODE(res));
            }
        }

        pBitmapFillStyle = pFillStyle;
        if (pBitmapFillStyle)
        {
            res = ExportBitmapFillStyle(pBitmapFillStyle);
            ASSERT(FCM_SUCCESS_CODE(res));
        }

        return res;
    }


    FCM::Result ResourcePalette::ExportStroke(DOM::FrameElement::PIShape pIShape)
    {
        FCM::FCMListPtr pStrokeGroupList;
        FCM::U_Int32 strokeStyleCount;
        FCM::Result res;

        FCM::AutoPtr<FCM::IFCMUnknown> pUnkSRVReg;
        GetCallback()->GetService(DOM::FLA_REGION_GENERATOR_SERVICE, pUnkSRVReg.m_Ptr);
        AutoPtr<DOM::Service::Shape::IRegionGeneratorService> pIRegionGeneratorService(pUnkSRVReg);
        ASSERT(pIRegionGeneratorService);

        res = pIRegionGeneratorService->GetStrokeGroups(pIShape, pStrokeGroupList.m_Ptr);
        ASSERT(FCM_SUCCESS_CODE(res));

        res = pStrokeGroupList->Count(strokeStyleCount);
        ASSERT(FCM_SUCCESS_CODE(res));

        AutoPtr<DOM::FillStyle::ISolidFillStyle> pSolidFillStyle = NULL;
        AutoPtr<DOM::FillStyle::IGradientFillStyle> pGradientFillStyle = NULL;
        AutoPtr<DOM::FillStyle::IBitmapFillStyle> pBitmapFillStyle = NULL;

        AutoPtr<FCM::IFCMUnknown> pGrad;
        for (FCM::U_Int32 j = 0; j < strokeStyleCount; j++)
        {
            AutoPtr<DOM::Service::Shape::IStrokeGroup> pStrokeGroup = pStrokeGroupList[j];
            ASSERT(pStrokeGroup);

            res = m_pOutputWriter->StartDefineStrokeGroup();
            ASSERT(FCM_SUCCESS_CODE(res));

            AutoPtr<FCM::IFCMUnknown> pStrokeStyle;
            pStrokeGroup->GetStrokeStyle(pStrokeStyle.m_Ptr);

            DOM::Utils::COLOR color = {};

            FCMListPtr pPathList;
            FCM::U_Int32 pathCount;

            res = pStrokeGroup->GetPaths(pPathList.m_Ptr);
            ASSERT(FCM_SUCCESS_CODE(res));

            res = pPathList->Count(pathCount);
            ASSERT(FCM_SUCCESS_CODE(res));

            for (FCM::U_Int32 k = 0; k < pathCount; k++)
            {
                FCM::AutoPtr<DOM::Service::Shape::IPath> pPath;

                pPath = pPathList[k];
                ASSERT(pPath);

                res = m_pOutputWriter->StartDefineStroke();
                ASSERT(FCM_SUCCESS_CODE(res));

                res = ExportStrokeStyle(pStrokeStyle);
                ASSERT(FCM_SUCCESS_CODE(res));

                res = ExportPath(pPath);
                ASSERT(FCM_SUCCESS_CODE(res));

                res = m_pOutputWriter->EndDefineStroke();
                ASSERT(FCM_SUCCESS_CODE(res));
            }

            res = m_pOutputWriter->EndDefineStrokeGroup();
            ASSERT(FCM_SUCCESS_CODE(res));
        }

        return res;
    }


    FCM::Result ResourcePalette::HasFancyStrokes(DOM::FrameElement::PIShape pShape, FCM::Boolean& hasFancy)
    {
        FCM::Result res;
        FCM::FCMListPtr pStrokeGroupList;
        FCM::U_Int32 strokeStyleCount;

        hasFancy = false;

        FCM::AutoPtr<FCM::IFCMUnknown> pUnkSRVReg;
        GetCallback()->GetService(DOM::FLA_REGION_GENERATOR_SERVICE, pUnkSRVReg.m_Ptr);
        AutoPtr<DOM::Service::Shape::IRegionGeneratorService> pIRegionGeneratorService(pUnkSRVReg);
        ASSERT(pIRegionGeneratorService);

        res = pIRegionGeneratorService->GetStrokeGroups(pShape, pStrokeGroupList.m_Ptr);
        ASSERT(FCM_SUCCESS_CODE(res));

        res = pStrokeGroupList->Count(strokeStyleCount);
        ASSERT(FCM_SUCCESS_CODE(res));

        for (FCM::U_Int32 j = 0; j < strokeStyleCount; j++)
        {

            AutoPtr<DOM::StrokeStyle::ISolidStrokeStyle> pSolidStrokeStyle;
            AutoPtr<DOM::Service::Shape::IStrokeGroup> pStrokeGroup = pStrokeGroupList[j];
            ASSERT(pStrokeGroup);

            AutoPtr<FCM::IFCMUnknown> pStrokeStyle;
            pStrokeGroup->GetStrokeStyle(pStrokeStyle.m_Ptr);

            pSolidStrokeStyle = pStrokeStyle;

            if (pSolidStrokeStyle)
            {
                FCM::AutoPtr<DOM::StrokeStyle::IStrokeWidth> pStrokeWidth;

                pSolidStrokeStyle->GetStrokeWidth(pStrokeWidth.m_Ptr);

                if (pStrokeWidth.m_Ptr)
                {
                    // Variable width stroke
                    hasFancy = true;
                    break;
                }
            }
            else
            {
                // Not a solid stroke (may be dashed, dotted etc..)
                hasFancy = true;
                break;
            }
        }

        return FCM_SUCCESS;
    }


    // Convert strokes to fills
    FCM::Result ResourcePalette::ConvertStrokeToFill(
        DOM::FrameElement::PIShape pShape,
        DOM::FrameElement::PIShape& pNewShape)
    {
        FCM::Result res;
        FCM::AutoPtr<FCM::IFCMUnknown> pUnkSRVReg;

        GetCallback()->GetService(DOM::FLA_SHAPE_SERVICE, pUnkSRVReg.m_Ptr);

        AutoPtr<DOM::Service::Shape::IShapeService> pIShapeService(pUnkSRVReg);
        ASSERT(pIShapeService);

        res = pIShapeService->ConvertStrokeToFill(pShape, pNewShape);
        ASSERT(FCM_SUCCESS_CODE(res));

        return FCM_SUCCESS;
    }

    FCM::Result ResourcePalette::ExportStrokeStyle(FCM::PIFCMUnknown pStrokeStyle)
    {
        FCM::Result res = FCM_SUCCESS;
        AutoPtr<DOM::StrokeStyle::ISolidStrokeStyle> pSolidStrokeStyle;

        pSolidStrokeStyle = pStrokeStyle;

        if (pSolidStrokeStyle)
        {
            res = ExportSolidStrokeStyle(pSolidStrokeStyle);
        }
        else
        {
            // Other stroke styles are not tested yet.
        }

        return res;
    }


    FCM::Result ResourcePalette::ExportSolidStrokeStyle(DOM::StrokeStyle::ISolidStrokeStyle* pSolidStrokeStyle)
    {
        FCM::Result res;
        FCM::Double thickness;
        AutoPtr<DOM::IFCMUnknown> pFillStyle;
        DOM::StrokeStyle::CAP_STYLE capStyle;
        DOM::StrokeStyle::JOIN_STYLE joinStyle;
        DOM::Utils::ScaleType scaleType;
        FCM::Boolean strokeHinting;


        capStyle.structSize = sizeof(DOM::StrokeStyle::CAP_STYLE);
        res = pSolidStrokeStyle->GetCapStyle(capStyle);
        ASSERT(FCM_SUCCESS_CODE(res));

        joinStyle.structSize = sizeof(DOM::StrokeStyle::JOIN_STYLE);
        res = pSolidStrokeStyle->GetJoinStyle(joinStyle);
        ASSERT(FCM_SUCCESS_CODE(res));

        res = pSolidStrokeStyle->GetThickness(thickness);
        ASSERT(FCM_SUCCESS_CODE(res));

        if (thickness < 0.1)
        {
            thickness = 0.1;
        }

        res = pSolidStrokeStyle->GetScaleType(scaleType);
        ASSERT(FCM_SUCCESS_CODE(res));

        res = pSolidStrokeStyle->GetStrokeHinting(strokeHinting);
        ASSERT(FCM_SUCCESS_CODE(res));

        res = m_pOutputWriter->StartDefineSolidStrokeStyle(
            thickness,
            joinStyle,
            capStyle,
            scaleType,
            strokeHinting);
        ASSERT(FCM_SUCCESS_CODE(res));

        // Stroke fill styles
        res = pSolidStrokeStyle->GetFillStyle(pFillStyle.m_Ptr);
        ASSERT(FCM_SUCCESS_CODE(res));

        res = ExportFillStyle(pFillStyle);
        ASSERT(FCM_SUCCESS_CODE(res));

        res = m_pOutputWriter->EndDefineSolidStrokeStyle();
        ASSERT(FCM_SUCCESS_CODE(res));

        return res;
    }


    FCM::Result ResourcePalette::ExportSolidFillStyle(DOM::FillStyle::ISolidFillStyle* pSolidFillStyle)
    {
        FCM::Result res;
        DOM::Utils::COLOR color;

        AutoPtr<DOM::FillStyle::ISolidFillStyle> solidFill = pSolidFillStyle;
        ASSERT(solidFill);

        res = solidFill->GetColor(color);
        ASSERT(FCM_SUCCESS_CODE(res));

        m_pOutputWriter->DefineSolidFillStyle(color);

        return res;
    }


    FCM::Result ResourcePalette::ExportRadialGradientFillStyle(DOM::FillStyle::IGradientFillStyle* pGradientFillStyle)
    {
        DOM::FillStyle::GradientSpread spread;

        AutoPtr<FCM::IFCMUnknown> pGrad;

        AutoPtr<DOM::FillStyle::IGradientFillStyle> gradientFill = pGradientFillStyle;
        FCM::Result res = gradientFill->GetSpread(spread);
        ASSERT(FCM_SUCCESS_CODE(res));

        res = gradientFill->GetColorGradient(pGrad.m_Ptr);
        ASSERT(FCM_SUCCESS_CODE(res));

        AutoPtr<DOM::Utils::IRadialColorGradient> radialColorGradient = pGrad;
        ASSERT(radialColorGradient);

        DOM::Utils::MATRIX2D matrix;
        res = gradientFill->GetMatrix(matrix);
        ASSERT(FCM_SUCCESS_CODE(res));

        FCM::S_Int32 focalPoint = 0;
        res = radialColorGradient->GetFocalPoint(focalPoint);
        ASSERT(FCM_SUCCESS_CODE(res));

        res = m_pOutputWriter->StartDefineRadialGradientFillStyle(spread, matrix, focalPoint);
        ASSERT(FCM_SUCCESS_CODE(res));

        FCM::U_Int8 nColors;
        res = radialColorGradient->GetKeyColorCount(nColors);
        ASSERT(FCM_SUCCESS_CODE(res));

        for (FCM::U_Int8 i = 0; i < nColors; i++)
        {
            DOM::Utils::GRADIENT_COLOR_POINT point;

            res = radialColorGradient->GetKeyColorAtIndex(i, point);
            ASSERT(FCM_SUCCESS_CODE(res));

            res = m_pOutputWriter->SetKeyColorPoint(point);
            ASSERT(FCM_SUCCESS_CODE(res));
        }

        res = m_pOutputWriter->EndDefineRadialGradientFillStyle();
        ASSERT(FCM_SUCCESS_CODE(res));

        return res;
    }


    FCM::Result ResourcePalette::ExportLinearGradientFillStyle(DOM::FillStyle::IGradientFillStyle* pGradientFillStyle)
    {
        DOM::FillStyle::GradientSpread spread;
        AutoPtr<FCM::IFCMUnknown> pGrad;

        AutoPtr<DOM::FillStyle::IGradientFillStyle> gradientFill = pGradientFillStyle;
        FCM::Result res = gradientFill->GetSpread(spread);
        ASSERT(FCM_SUCCESS_CODE(res));

        res = gradientFill->GetColorGradient(pGrad.m_Ptr);
        ASSERT(FCM_SUCCESS_CODE(res));

        AutoPtr<DOM::Utils::ILinearColorGradient> linearColorGradient = pGrad;
        ASSERT(linearColorGradient);

        DOM::Utils::MATRIX2D matrix;
        res = gradientFill->GetMatrix(matrix);
        ASSERT(FCM_SUCCESS_CODE(res));

        res = m_pOutputWriter->StartDefineLinearGradientFillStyle(spread, matrix);
        ASSERT(FCM_SUCCESS_CODE(res));

        FCM::U_Int8 nColors;
        res = linearColorGradient->GetKeyColorCount(nColors);
        ASSERT(FCM_SUCCESS_CODE(res));

        for (FCM::U_Int8 i = 0; i < nColors; i++)
        {
            DOM::Utils::GRADIENT_COLOR_POINT point;

            res = linearColorGradient->GetKeyColorAtIndex(i, point);
            ASSERT(FCM_SUCCESS_CODE(res));

            res = m_pOutputWriter->SetKeyColorPoint(point);
            ASSERT(FCM_SUCCESS_CODE(res));
        }

        res = m_pOutputWriter->EndDefineLinearGradientFillStyle();
        ASSERT(FCM_SUCCESS_CODE(res));

        return res;
    }


    FCM::Result ResourcePalette::ExportBitmapFillStyle(DOM::FillStyle::IBitmapFillStyle* pBitmapFillStyle)
    {
        DOM::AutoPtr<DOM::ILibraryItem> pLibItem;
        DOM::AutoPtr<DOM::LibraryItem::IMediaItem> pMediaItem;
        FCM::Result res;
        FCM::Boolean isClipped;
        DOM::Utils::MATRIX2D matrix;
        std::string name;
        FCM::StringRep16 pName;

        // IsClipped ?
        res = pBitmapFillStyle->IsClipped(isClipped);
        ASSERT(FCM_SUCCESS_CODE(res));

        // Matrix
        res = pBitmapFillStyle->GetMatrix(matrix);
        ASSERT(FCM_SUCCESS_CODE(res));

        // Get name
        res = pBitmapFillStyle->GetBitmap(pMediaItem.m_Ptr);
        ASSERT(FCM_SUCCESS_CODE(res));

        pLibItem = pMediaItem;

        AutoPtr<FCM::IFCMUnknown> medInfo;
        pMediaItem->GetMediaInfo(medInfo.m_Ptr);

        AutoPtr<DOM::MediaInfo::IBitmapInfo> bitsInfo = medInfo;
        ASSERT(bitsInfo);

        // Get image height
        FCM::S_Int32 height;
        res = bitsInfo->GetHeight(height);
        ASSERT(FCM_SUCCESS_CODE(res));

        // Store the resource name
        res = pLibItem->GetName(&pName);
        ASSERT(FCM_SUCCESS_CODE(res));
        std::string libItemName = Utils::ToString(pName, GetCallback());
        m_resourceNames.push_back(libItemName);

        // Get image width
        FCM::S_Int32 width;
        res = bitsInfo->GetWidth(width);
        ASSERT(FCM_SUCCESS_CODE(res));

        // Dump the definition of a bitmap fill style
        res = m_pOutputWriter->DefineBitmapFillStyle(
            isClipped,
            matrix,
            height,
            width,
            libItemName,
            pMediaItem);
        ASSERT(FCM_SUCCESS_CODE(res));

        // Free the name
        FCM::AutoPtr<FCM::IFCMUnknown> pUnkCalloc;
        res = GetCallback()->GetService(SRVCID_Core_Memory, pUnkCalloc.m_Ptr);
        AutoPtr<FCM::IFCMCalloc> callocService = pUnkCalloc;

        callocService->Free((FCM::PVoid)pName);

        return res;
    }


    FCM::Result ResourcePalette::GetFontInfo(DOM::FrameElement::ITextStyle* pTextStyleItem, std::string& name, FCM::U_Int16 fontSize)
    {
        FCM::StringRep16 pFontName;
        FCM::StringRep8 pFontStyle;
        FCM::Result res;
        std::string str;
        std::string sizeStr;
        std::string styleStr;

        res = pTextStyleItem->GetFontName(&pFontName);
        ASSERT(FCM_SUCCESS_CODE(res));

        res = pTextStyleItem->GetFontStyle(&pFontStyle);
        ASSERT(FCM_SUCCESS_CODE(res));

        styleStr = pFontStyle;
        if (styleStr == "BoldItalicStyle")
            styleStr = "italic bold";
        else if (styleStr == "BoldStyle")
            styleStr = "bold";
        else if (styleStr == "ItalicStyle")
            styleStr = "italic";
        else if (styleStr == "RegularStyle")
            styleStr = "";

        sizeStr = Utils::ToString(fontSize);
        str = Utils::ToString(pFontName, GetCallback());
        name = styleStr + " " + sizeStr + "px" + " " + "'" + str + "'";

        // Free the name and style
        FCM::AutoPtr<FCM::IFCMUnknown> pUnkCalloc;
        res = GetCallback()->GetService(SRVCID_Core_Memory, pUnkCalloc.m_Ptr);
        AutoPtr<FCM::IFCMCalloc> callocService = pUnkCalloc;

        callocService->Free((FCM::PVoid)pFontName);
        callocService->Free((FCM::PVoid)pFontStyle);

        return res;
    }
}