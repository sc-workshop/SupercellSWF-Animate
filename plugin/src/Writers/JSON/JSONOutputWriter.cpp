#include "Writers/JSON/JSONOutputWriter.h"
#include "Writers/JSON/JSONTimelineWriter.h"

#include <ApplicationFCMPublicIDs.h>
#include <Service/Image/IBitmapExportService.h>
#include <Service/Sound/ISoundExportService.h>

#include "Utils.h"

namespace SupercellSWF {
    static const std::string moveTo = "M";
    static const std::string lineTo = "L";
    static const std::string bezierCurveTo = "Q";
    static const std::string space = " ";
    static const std::string comma = ",";
    static const std::string semiColon = ";";

    static const FCM::Float GRADIENT_VECTOR_CONSTANT = 16384.0;

    FCM::Result JSONOutputWriter::StartOutput(std::string& outputFileName)
    {
        std::string parent;
        std::string jsonFile;

        Utils::GetParent(outputFileName, parent);
        Utils::GetFileNameWithoutExtension(outputFileName, jsonFile);
        m_outputJSONFileName = jsonFile + ".json";
        m_outputJSONFilePath = parent + jsonFile + ".json";
        m_outputImageFolder = parent + IMAGE_FOLDER;
        m_outputSoundFolder = parent + SOUND_FOLDER;

        return FCM_SUCCESS;
    }


    FCM::Result JSONOutputWriter::EndOutput()
    {

        return FCM_SUCCESS;
    }


    FCM::Result JSONOutputWriter::StartDocument(
        const DOM::Utils::COLOR& background,
        FCM::U_Int32 stageHeight,
        FCM::U_Int32 stageWidth,
        FCM::U_Int32 fps)
    {
        return FCM_SUCCESS;
    }


    FCM::Result JSONOutputWriter::EndDocument()
    {
        std::fstream file;
        m_pRootNode->push_back(*m_pShapeArray);
        m_pRootNode->push_back(*m_pBitmapArray);
        m_pRootNode->push_back(*m_pSoundArray);
        m_pRootNode->push_back(*m_pTextArray);
        m_pRootNode->push_back(*m_pTimelineArray);

        // Write the JSON file (overwrite file if it already exists)
        Utils::OpenFStream(m_outputJSONFilePath, file, std::ios_base::trunc | std::ios_base::out, m_pCallback);

        JSONNode firstNode(JSON_NODE);
        firstNode.push_back(*m_pRootNode);

        file << firstNode.write_formatted();
        file.close();

        return FCM_SUCCESS;
    }


    FCM::Result JSONOutputWriter::StartDefineTimeline()
    {
        return FCM_SUCCESS;
    }


    FCM::Result JSONOutputWriter::EndDefineTimeline(
        FCM::U_Int32 resId,
        FCM::StringRep16 pName,
        TimelineWriter* pTimelineWriter)
    {
        JSONTimelineWriter* pWriter = static_cast<JSONTimelineWriter*> (pTimelineWriter);

        pWriter->Finish(resId, pName);

        m_pTimelineArray->push_back(*(pWriter->GetRoot()));

        return FCM_SUCCESS;
    }


    FCM::Result JSONOutputWriter::StartDefineShape()
    {
        m_shapeElem = new JSONNode(JSON_NODE);
        ASSERT(m_shapeElem);

        m_pathArray = new JSONNode(JSON_ARRAY);
        ASSERT(m_pathArray);
        m_pathArray->set_name("path");

        return FCM_SUCCESS;
    }


    // Marks the end of a shape
    FCM::Result JSONOutputWriter::EndDefineShape(FCM::U_Int32 resId)
    {
        m_shapeElem->push_back(JSONNode(("charid"), SupercellSWF::Utils::ToString(resId)));
        m_shapeElem->push_back(*m_pathArray);

        m_pShapeArray->push_back(*m_shapeElem);

        delete m_pathArray;
        delete m_shapeElem;

        return FCM_SUCCESS;
    }


    // Start of fill region definition
    FCM::Result JSONOutputWriter::StartDefineFill()
    {
        m_pathElem = new JSONNode(JSON_NODE);
        ASSERT(m_pathElem);

        m_pathCmdStr.clear();

        return FCM_SUCCESS;
    }


    // Solid fill style definition
    FCM::Result JSONOutputWriter::DefineSolidFillStyle(const DOM::Utils::COLOR& color)
    {
        std::string colorStr = Utils::ToString(color);
        std::string colorOpacityStr = SupercellSWF::Utils::ToString((double)(color.alpha / 255.0));

        m_pathElem->push_back(JSONNode("color", colorStr.c_str()));
        m_pathElem->push_back(JSONNode("colorOpacity", colorOpacityStr.c_str()));

        return FCM_SUCCESS;
    }


    // Bitmap fill style definition
    FCM::Result JSONOutputWriter::DefineBitmapFillStyle(
        FCM::Boolean clipped,
        const DOM::Utils::MATRIX2D& matrix,
        FCM::S_Int32 height,
        FCM::S_Int32 width,
        const std::string& libPathName,
        DOM::LibraryItem::PIMediaItem pMediaItem)
    {
        FCM::Result res;
        std::string name;
        JSONNode bitmapElem(JSON_NODE);
        std::string bitmapPath;
        std::string bitmapName;

        bitmapElem.set_name("image");

        bitmapElem.push_back(JSONNode(("height"), SupercellSWF::Utils::ToString(height)));
        bitmapElem.push_back(JSONNode(("width"), SupercellSWF::Utils::ToString(width)));

        FCM::AutoPtr<FCM::IFCMUnknown> pUnk;
        std::string bitmapRelPath;
        std::string bitmapExportPath = m_outputImageFolder + "/";

        FCM::Boolean alreadyExported = GetImageExportFileName(libPathName, name);
        if (!alreadyExported)
        {
            if (!m_imageFolderCreated)
            {
                res = Utils::CreateDir(m_outputImageFolder, m_pCallback);
                if (!(FCM_SUCCESS_CODE(res)))
                {
                    Utils::Trace(m_pCallback, "Output image folder (%s) could not be created\n", m_outputImageFolder.c_str());
                    return res;
                }
                m_imageFolderCreated = true;
            }
            CreateImageFileName(libPathName, name);
            SetImageExportFileName(libPathName, name);
        }

        bitmapExportPath += name;

        bitmapRelPath = "./";
        bitmapRelPath += IMAGE_FOLDER;
        bitmapRelPath += "/";
        bitmapRelPath += name;

        res = m_pCallback->GetService(DOM::FLA_BITMAP_SERVICE, pUnk.m_Ptr);
        ASSERT(FCM_SUCCESS_CODE(res));

        FCM::AutoPtr<DOM::Service::Image::IBitmapExportService> bitmapExportService = pUnk;
        if (bitmapExportService)
        {
            FCM::AutoPtr<FCM::IFCMCalloc> pCalloc;
            FCM::StringRep16 pFilePath = Utils::ToString16(bitmapExportPath, m_pCallback);
            res = bitmapExportService->ExportToFile(pMediaItem, pFilePath, 100);
            ASSERT(FCM_SUCCESS_CODE(res));

            pCalloc = SupercellSWF::Utils::GetCallocService(m_pCallback);
            ASSERT(pCalloc.m_Ptr != NULL);

            pCalloc->Free(pFilePath);
        }

        bitmapElem.push_back(JSONNode(("bitmapPath"), bitmapRelPath));

        DOM::Utils::MATRIX2D matrix1 = matrix;
        matrix1.a /= 20.0;
        matrix1.b /= 20.0;
        matrix1.c /= 20.0;
        matrix1.d /= 20.0;

        bitmapElem.push_back(JSONNode(("patternUnits"), "userSpaceOnUse"));
        bitmapElem.push_back(JSONNode(("patternTransform"), Utils::ToString(matrix1).c_str()));

        m_pathElem->push_back(bitmapElem);

        return FCM_SUCCESS;
    }


    // Start Linear Gradient fill style definition
    FCM::Result JSONOutputWriter::StartDefineLinearGradientFillStyle(
        DOM::FillStyle::GradientSpread spread,
        const DOM::Utils::MATRIX2D& matrix)
    {
        DOM::Utils::POINT2D point;

        m_gradientColor = new JSONNode(JSON_NODE);
        ASSERT(m_gradientColor);
        m_gradientColor->set_name("linearGradient");

        point.x = -GRADIENT_VECTOR_CONSTANT / 20;
        point.y = 0;
        Utils::TransformPoint(matrix, point, point);

        m_gradientColor->push_back(JSONNode("x1", Utils::ToString((double)(point.x))));
        m_gradientColor->push_back(JSONNode("y1", Utils::ToString((double)(point.y))));

        point.x = GRADIENT_VECTOR_CONSTANT / 20;
        point.y = 0;
        Utils::TransformPoint(matrix, point, point);

        m_gradientColor->push_back(JSONNode("x2", Utils::ToString((double)(point.x))));
        m_gradientColor->push_back(JSONNode("y2", Utils::ToString((double)(point.y))));

        m_gradientColor->push_back(JSONNode("spreadMethod", Utils::ToString(spread)));

        m_stopPointArray = new JSONNode(JSON_ARRAY);
        ASSERT(m_stopPointArray);
        m_stopPointArray->set_name("stop");

        return FCM_SUCCESS;
    }


    // Sets a specific key point in a color ramp (for both radial and linear gradient)
    FCM::Result JSONOutputWriter::SetKeyColorPoint(
        const DOM::Utils::GRADIENT_COLOR_POINT& colorPoint)
    {
        JSONNode stopEntry(JSON_NODE);
        FCM::Float offset;

        offset = (float)((colorPoint.pos * 100) / 255.0);

        stopEntry.push_back(JSONNode("offset", Utils::ToString((double)offset)));
        stopEntry.push_back(JSONNode("stopColor", Utils::ToString(colorPoint.color)));
        stopEntry.push_back(JSONNode("stopOpacity", Utils::ToString((double)(colorPoint.color.alpha / 255.0))));

        m_stopPointArray->push_back(stopEntry);

        return FCM_SUCCESS;
    }


    // End Linear Gradient fill style definition
    FCM::Result JSONOutputWriter::EndDefineLinearGradientFillStyle()
    {
        m_gradientColor->push_back(*m_stopPointArray);
        m_pathElem->push_back(*m_gradientColor);

        delete m_stopPointArray;
        delete m_gradientColor;

        return FCM_SUCCESS;
    }


    // Start Radial Gradient fill style definition
    FCM::Result JSONOutputWriter::StartDefineRadialGradientFillStyle(
        DOM::FillStyle::GradientSpread spread,
        const DOM::Utils::MATRIX2D& matrix,
        FCM::S_Int32 focalPoint)
    {
        DOM::Utils::POINT2D point;
        DOM::Utils::POINT2D point1;
        DOM::Utils::POINT2D point2;

        m_gradientColor = new JSONNode(JSON_NODE);
        ASSERT(m_gradientColor);
        m_gradientColor->set_name("radialGradient");

        point.x = 0;
        point.y = 0;
        Utils::TransformPoint(matrix, point, point1);

        point.x = GRADIENT_VECTOR_CONSTANT / 20;
        point.y = 0;
        Utils::TransformPoint(matrix, point, point2);

        FCM::Float xd = point1.x - point2.x;
        FCM::Float yd = point1.y - point2.y;
        FCM::Float r = sqrt(xd * xd + yd * yd);

        FCM::Float angle = atan2(yd, xd);
        double focusPointRatio = focalPoint / 255.0;
        double fx = -r * focusPointRatio * cos(angle);
        double fy = -r * focusPointRatio * sin(angle);

        m_gradientColor->push_back(JSONNode("cx", "0"));
        m_gradientColor->push_back(JSONNode("cy", "0"));
        m_gradientColor->push_back(JSONNode("r", Utils::ToString((double)r)));
        m_gradientColor->push_back(JSONNode("fx", Utils::ToString((double)fx)));
        m_gradientColor->push_back(JSONNode("fy", Utils::ToString((double)fy)));

        FCM::Float scaleFactor = (GRADIENT_VECTOR_CONSTANT / 20) / r;
        DOM::Utils::MATRIX2D matrix1 = {};
        matrix1.a = matrix.a * scaleFactor;
        matrix1.b = matrix.b * scaleFactor;
        matrix1.c = matrix.c * scaleFactor;
        matrix1.d = matrix.d * scaleFactor;
        matrix1.tx = matrix.tx;
        matrix1.ty = matrix.ty;

        m_gradientColor->push_back(JSONNode("gradientTransform", Utils::ToString(matrix1)));
        m_gradientColor->push_back(JSONNode("spreadMethod", Utils::ToString(spread)));

        m_stopPointArray = new JSONNode(JSON_ARRAY);
        ASSERT(m_stopPointArray);
        m_stopPointArray->set_name("stop");

        return FCM_SUCCESS;
    }


    // End Radial Gradient fill style definition
    FCM::Result JSONOutputWriter::EndDefineRadialGradientFillStyle()
    {
        m_gradientColor->push_back(*m_stopPointArray);
        m_pathElem->push_back(*m_gradientColor);

        delete m_stopPointArray;
        delete m_gradientColor;

        return FCM_SUCCESS;
    }


    // Start of fill region boundary
    FCM::Result JSONOutputWriter::StartDefineBoundary()
    {
        return StartDefinePath();
    }


    // Sets a segment of a path (Used for boundary, holes)
    FCM::Result JSONOutputWriter::SetSegment(const DOM::Utils::SEGMENT& segment)
    {
        if (m_firstSegment)
        {
            if (segment.segmentType == DOM::Utils::LINE_SEGMENT)
            {
                m_pathCmdStr.append(SupercellSWF::Utils::ToString((double)(segment.line.endPoint1.x)));
                m_pathCmdStr.append(space);
                m_pathCmdStr.append(SupercellSWF::Utils::ToString((double)(segment.line.endPoint1.y)));
                m_pathCmdStr.append(space);
            }
            else
            {
                m_pathCmdStr.append(SupercellSWF::Utils::ToString((double)(segment.quadBezierCurve.anchor1.x)));
                m_pathCmdStr.append(space);
                m_pathCmdStr.append(SupercellSWF::Utils::ToString((double)(segment.quadBezierCurve.anchor1.y)));
                m_pathCmdStr.append(space);
            }
            m_firstSegment = false;
        }

        if (segment.segmentType == DOM::Utils::LINE_SEGMENT)
        {
            m_pathCmdStr.append(lineTo);
            m_pathCmdStr.append(space);
            m_pathCmdStr.append(SupercellSWF::Utils::ToString((double)(segment.line.endPoint2.x)));
            m_pathCmdStr.append(space);
            m_pathCmdStr.append(SupercellSWF::Utils::ToString((double)(segment.line.endPoint2.y)));
            m_pathCmdStr.append(space);
        }
        else
        {
            m_pathCmdStr.append(bezierCurveTo);
            m_pathCmdStr.append(space);
            m_pathCmdStr.append(SupercellSWF::Utils::ToString((double)(segment.quadBezierCurve.control.x)));
            m_pathCmdStr.append(space);
            m_pathCmdStr.append(SupercellSWF::Utils::ToString((double)(segment.quadBezierCurve.control.y)));
            m_pathCmdStr.append(space);
            m_pathCmdStr.append(SupercellSWF::Utils::ToString((double)(segment.quadBezierCurve.anchor2.x)));
            m_pathCmdStr.append(space);
            m_pathCmdStr.append(SupercellSWF::Utils::ToString((double)(segment.quadBezierCurve.anchor2.y)));
            m_pathCmdStr.append(space);
        }

        return FCM_SUCCESS;
    }


    // End of fill region boundary
    FCM::Result JSONOutputWriter::EndDefineBoundary()
    {
        return EndDefinePath();
    }


    // Start of fill region hole
    FCM::Result JSONOutputWriter::StartDefineHole()
    {
        return StartDefinePath();
    }


    // End of fill region hole
    FCM::Result JSONOutputWriter::EndDefineHole()
    {
        return EndDefinePath();
    }


    // Start of stroke group
    FCM::Result JSONOutputWriter::StartDefineStrokeGroup()
    {
        // No need to do anything
        return FCM_SUCCESS;
    }


    // Start solid stroke style definition
    FCM::Result JSONOutputWriter::StartDefineSolidStrokeStyle(
        FCM::Double thickness,
        const DOM::StrokeStyle::JOIN_STYLE& joinStyle,
        const DOM::StrokeStyle::CAP_STYLE& capStyle,
        DOM::Utils::ScaleType scaleType,
        FCM::Boolean strokeHinting)
    {
        m_strokeStyle.type = SOLID_STROKE_STYLE_TYPE;
        m_strokeStyle.solidStrokeStyle.capStyle = capStyle;
        m_strokeStyle.solidStrokeStyle.joinStyle = joinStyle;
        m_strokeStyle.solidStrokeStyle.thickness = thickness;
        m_strokeStyle.solidStrokeStyle.scaleType = scaleType;
        m_strokeStyle.solidStrokeStyle.strokeHinting = strokeHinting;

        return FCM_SUCCESS;
    }


    // End of solid stroke style
    FCM::Result JSONOutputWriter::EndDefineSolidStrokeStyle()
    {
        // No need to do anything
        return FCM_SUCCESS;
    }


    // Start of stroke 
    FCM::Result JSONOutputWriter::StartDefineStroke()
    {
        m_pathElem = new JSONNode(JSON_NODE);
        ASSERT(m_pathElem);

        m_pathCmdStr.clear();
        StartDefinePath();

        return FCM_SUCCESS;
    }


    // End of a stroke 
    FCM::Result JSONOutputWriter::EndDefineStroke()
    {
        m_pathElem->push_back(JSONNode("d", m_pathCmdStr));

        if (m_strokeStyle.type == SOLID_STROKE_STYLE_TYPE)
        {
            m_pathElem->push_back(JSONNode("strokeWidth", SupercellSWF::Utils::ToString((double)m_strokeStyle.solidStrokeStyle.thickness).c_str()));
            m_pathElem->push_back(JSONNode("fill", "none"));
            m_pathElem->push_back(JSONNode("strokeLinecap", Utils::ToString(m_strokeStyle.solidStrokeStyle.capStyle.type).c_str()));
            m_pathElem->push_back(JSONNode("strokeLinejoin", Utils::ToString(m_strokeStyle.solidStrokeStyle.joinStyle.type).c_str()));

            if (m_strokeStyle.solidStrokeStyle.joinStyle.type == DOM::Utils::MITER_JOIN)
            {
                m_pathElem->push_back(JSONNode(
                    "stroke-miterlimit",
                    SupercellSWF::Utils::ToString((double)m_strokeStyle.solidStrokeStyle.joinStyle.miterJoinProp.miterLimit).c_str()));
            }
            m_pathElem->push_back(JSONNode("pathType", "Stroke"));
        }
        m_pathArray->push_back(*m_pathElem);

        delete m_pathElem;

        m_pathElem = NULL;

        return FCM_SUCCESS;
    }


    // End of stroke group
    FCM::Result JSONOutputWriter::EndDefineStrokeGroup()
    {
        // No need to do anything
        return FCM_SUCCESS;
    }


    // End of fill style definition
    FCM::Result JSONOutputWriter::EndDefineFill()
    {
        m_pathElem->push_back(JSONNode("d", m_pathCmdStr));
        m_pathElem->push_back(JSONNode("pathType", JSON_TEXT("Fill")));
        m_pathElem->push_back(JSONNode("stroke", JSON_TEXT("none")));

        m_pathArray->push_back(*m_pathElem);

        delete m_pathElem;

        m_pathElem = NULL;

        return FCM_SUCCESS;
    }


    // Define a bitmap
    FCM::Result JSONOutputWriter::DefineBitmap(
        FCM::U_Int32 resId,
        FCM::S_Int32 height,
        FCM::S_Int32 width,
        const std::string& libPathName,
        DOM::LibraryItem::PIMediaItem pMediaItem)
    {
        FCM::Result res;
        JSONNode bitmapElem(JSON_NODE);
        std::string bitmapPath;
        std::string bitmapName;
        std::string name;

        bitmapElem.set_name("image");

        bitmapElem.push_back(JSONNode(("charid"), SupercellSWF::Utils::ToString(resId)));
        bitmapElem.push_back(JSONNode(("height"), SupercellSWF::Utils::ToString(height)));
        bitmapElem.push_back(JSONNode(("width"), SupercellSWF::Utils::ToString(width)));

        FCM::AutoPtr<FCM::IFCMUnknown> pUnk;
        std::string bitmapRelPath;
        std::string bitmapExportPath = m_outputImageFolder + "/";

        FCM::Boolean alreadyExported = GetImageExportFileName(libPathName, name);
        if (!alreadyExported)
        {
            if (!m_imageFolderCreated)
            {
                res = Utils::CreateDir(m_outputImageFolder, m_pCallback);
                if (!(FCM_SUCCESS_CODE(res)))
                {
                    Utils::Trace(m_pCallback, "Output image folder (%s) could not be created\n", m_outputImageFolder.c_str());
                    return res;
                }
                m_imageFolderCreated = true;
            }
            CreateImageFileName(libPathName, name);
            SetImageExportFileName(libPathName, name);
        }

        bitmapExportPath += name;

        bitmapRelPath = "./";
        bitmapRelPath += IMAGE_FOLDER;
        bitmapRelPath += "/";
        bitmapRelPath += name;

        res = m_pCallback->GetService(DOM::FLA_BITMAP_SERVICE, pUnk.m_Ptr);
        ASSERT(FCM_SUCCESS_CODE(res));

        FCM::AutoPtr<DOM::Service::Image::IBitmapExportService> bitmapExportService = pUnk;
        if (bitmapExportService)
        {
            FCM::AutoPtr<FCM::IFCMCalloc> pCalloc;
            FCM::StringRep16 pFilePath = Utils::ToString16(bitmapExportPath, m_pCallback);
            res = bitmapExportService->ExportToFile(pMediaItem, pFilePath, 100);
            ASSERT(FCM_SUCCESS_CODE(res));

            pCalloc = SupercellSWF::Utils::GetCallocService(m_pCallback);
            ASSERT(pCalloc.m_Ptr != NULL);

            pCalloc->Free(pFilePath);
        }

        bitmapElem.push_back(JSONNode(("bitmapPath"), bitmapRelPath));

        m_pBitmapArray->push_back(bitmapElem);

        return FCM_SUCCESS;
    }

    FCM::Result JSONOutputWriter::DefineText(
        FCM::U_Int32 resId,
        const std::string& name,
        const DOM::Utils::COLOR& color,
        const std::string& displayText,
        DOM::FrameElement::PIClassicText pTextItem)
    {
        std::string txt = displayText;
        std::string colorStr = Utils::ToString(color);
        std::string find = "\r";
        std::string replace = "\\r";
        std::string::size_type i = 0;
        JSONNode textElem(JSON_NODE);

        while (true) {
            /* Locate the substring to replace. */
            i = txt.find(find, i);

            if (i == std::string::npos) break;
            /* Make the replacement. */
            txt.replace(i, find.length(), replace);

            /* Advance index forward so the next iteration doesn't pick it up as well. */
            i += replace.length();
        }


        textElem.push_back(JSONNode(("charid"), SupercellSWF::Utils::ToString(resId)));
        textElem.push_back(JSONNode(("displayText"), txt));
        textElem.push_back(JSONNode(("font"), name));
        textElem.push_back(JSONNode("color", colorStr.c_str()));

        m_pTextArray->push_back(textElem);

        return FCM_SUCCESS;
    }

    FCM::Result JSONOutputWriter::DefineSound(
        FCM::U_Int32 resId,
        const std::string& libPathName,
        DOM::LibraryItem::PIMediaItem pMediaItem)
    {
        FCM::Result res;
        JSONNode soundElem(JSON_NODE);
        std::string soundPath;
        std::string soundName;
        std::string name;

        soundElem.set_name("sound");
        soundElem.push_back(JSONNode(("charid"), SupercellSWF::Utils::ToString(resId)));

        FCM::AutoPtr<FCM::IFCMUnknown> pUnk;
        std::string soundRelPath;
        std::string soundExportPath = m_outputSoundFolder + "/";

        if (!m_soundFolderCreated)
        {
            res = Utils::CreateDir(m_outputSoundFolder, m_pCallback);
            if (!(FCM_SUCCESS_CODE(res)))
            {
                Utils::Trace(m_pCallback, "Output sound folder (%s) could not be created\n", m_outputSoundFolder.c_str());
                return res;
            }
            m_soundFolderCreated = true;
        }

        CreateSoundFileName(libPathName, name);
        soundExportPath += name;

        soundRelPath = "./";
        soundRelPath += SOUND_FOLDER;
        soundRelPath += "/";
        soundRelPath += name;

        res = m_pCallback->GetService(DOM::FLA_SOUND_SERVICE, pUnk.m_Ptr);
        ASSERT(FCM_SUCCESS_CODE(res));
        FCM::AutoPtr<DOM::Service::Sound::ISoundExportService> soundExportService = pUnk;
        if (soundExportService)
        {
            FCM::AutoPtr<FCM::IFCMCalloc> pCalloc;
            FCM::StringRep16 pFilePath = Utils::ToString16(soundExportPath, m_pCallback);
            res = soundExportService->ExportToFile(pMediaItem, pFilePath);
            ASSERT(FCM_SUCCESS_CODE(res));
            pCalloc = SupercellSWF::Utils::GetCallocService(m_pCallback);
            ASSERT(pCalloc.m_Ptr != NULL);
            pCalloc->Free(pFilePath);
        }

        soundElem.push_back(JSONNode(("soundPath"), soundRelPath));
        m_pSoundArray->push_back(soundElem);

        return FCM_SUCCESS;
    }

    JSONOutputWriter::JSONOutputWriter(FCM::PIFCMCallback pCallback)
        : m_pCallback(pCallback),
        m_shapeElem(NULL),
        m_pathArray(NULL),
        m_pathElem(NULL),
        m_firstSegment(false),
        m_imageFileNameLabel(0),
        m_soundFileNameLabel(0),
        m_imageFolderCreated(false),
        m_soundFolderCreated(false)
    {
        m_pRootNode = new JSONNode(JSON_NODE);
        ASSERT(m_pRootNode);
        m_pRootNode->set_name("DOMDocument");

        m_pShapeArray = new JSONNode(JSON_ARRAY);
        ASSERT(m_pShapeArray);
        m_pShapeArray->set_name("Shape");

        m_pTimelineArray = new JSONNode(JSON_ARRAY);
        ASSERT(m_pTimelineArray);
        m_pTimelineArray->set_name("Timeline");

        m_pBitmapArray = new JSONNode(JSON_ARRAY);
        ASSERT(m_pBitmapArray);
        m_pBitmapArray->set_name("Bitmaps");

        m_pTextArray = new JSONNode(JSON_ARRAY);
        ASSERT(m_pTextArray);
        m_pTextArray->set_name("Text");

        m_pSoundArray = new JSONNode(JSON_ARRAY);
        ASSERT(m_pSoundArray);
        m_pSoundArray->set_name("Sounds");
        m_strokeStyle.type = INVALID_STROKE_STYLE_TYPE;
    }


    JSONOutputWriter::~JSONOutputWriter()
    {
        delete m_pBitmapArray;
        delete m_pSoundArray;

        delete m_pTimelineArray;

        delete m_pShapeArray;

        delete m_pTextArray;

        delete m_pRootNode;
    }


    FCM::Result JSONOutputWriter::StartDefinePath()
    {
        m_pathCmdStr.append(moveTo);
        m_pathCmdStr.append(space);
        m_firstSegment = true;

        return FCM_SUCCESS;
    }

    FCM::Result JSONOutputWriter::EndDefinePath()
    {
        // No need to do anything
        return FCM_SUCCESS;
    }

    FCM::Result JSONOutputWriter::CreateImageFileName(const std::string& libPathName, std::string& name)
    {
        std::string str;
        size_t pos;
        std::string fileLabel;

        fileLabel = Utils::ToString(m_imageFileNameLabel);
        name = "Image" + fileLabel;
        m_imageFileNameLabel++;

        str = libPathName;

        // DOM APIs do not provide a way to get the compression of the image.
        // For time being, we will use the extension of the library item name.
        pos = str.rfind(".");
        if (pos != std::string::npos)
        {
            if (str.substr(pos + 1) == "jpg")
            {
                name += ".jpg";
            }
            else if (str.substr(pos + 1) == "png")
            {
                name += ".png";
            }
            else
            {
                name += ".png";
            }
        }
        else
        {
            name += ".png";
        }

        return FCM_SUCCESS;
    }


    FCM::Result JSONOutputWriter::CreateSoundFileName(const std::string& libPathName, std::string& name)
    {
        std::string str;
        size_t pos;
        std::string fileLabel;

        fileLabel = Utils::ToString(m_soundFileNameLabel);
        name = "Sound" + fileLabel;
        m_soundFileNameLabel++;

        str = libPathName;

        // DOM APIs do not provide a way to get the compression of the sound.
        // For time being, we will use the extension of the library item name.
        pos = str.rfind(".");
        if (pos != std::string::npos)
        {
            if (str.substr(pos + 1) == "wav")
            {
                name += ".WAV";
            }
            else if (str.substr(pos + 1) == "mp3")
            {
                name += ".MP3";
            }
            else
            {
                name += ".MP3";
            }
        }
        else
        {
            name += ".MP3";
        }

        return FCM_SUCCESS;
    }


    FCM::Boolean JSONOutputWriter::GetImageExportFileName(const std::string& libPathName, std::string& name)
    {
        std::map<std::string, std::string>::iterator it = m_imageMap.find(libPathName);

        name = "";

        if (it != m_imageMap.end())
        {
            // Image already exported
            name = it->second;
            return true;
        }

        return false;
    }


    void JSONOutputWriter::SetImageExportFileName(const std::string& libPathName, const std::string& name)
    {
        // Assumption: Name is not already present in the map
        ASSERT(m_imageMap.find(libPathName) == m_imageMap.end());

        m_imageMap.insert(std::pair<std::string, std::string>(libPathName, name));
    }
}