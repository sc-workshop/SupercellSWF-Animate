#pragma once

#include <FCMTypes.h>
#include <FCMPluginInterface.h>
#include <Exporter/Service/IResourcePalette.h>
#include <FillStyle/ISolidFillStyle.h>
#include <FillStyle/IBitmapFillStyle.h>
#include <FrameElement/ITextStyle.h>
#include <Service/Shape/IPath.h>

#include <string>
#include <vector>

#include "Writers/Base/OutputWriter.h"
#include "Version.h"

using namespace Exporter::Service;

namespace DOM
{
    namespace Service
    {
        namespace Shape
        {
            FORWARD_DECLARE_INTERFACE(IPath);
        }
    };
};

namespace SupercellSWF {
    class ResourcePalette : public IResourcePalette, public FCMObjectBase
    {
    public:

        BEGIN_INTERFACE_MAP(ResourcePalette, PLUGIN_VERSION)
            INTERFACE_ENTRY(IResourcePalette)
        END_INTERFACE_MAP

        virtual FCM::Result _FCMCALL AddSymbol(
            FCM::U_Int32 resourceId,
            FCM::StringRep16 pName,
            Exporter::Service::PITimelineBuilder pTimelineBuilder);

        virtual FCM::Result _FCMCALL AddShape(
            FCM::U_Int32 resourceId,
            DOM::FrameElement::PIShape pShape);

        virtual FCM::Result _FCMCALL AddSound(
            FCM::U_Int32 resourceId,
            DOM::LibraryItem::PIMediaItem pLibItem);

        virtual FCM::Result _FCMCALL AddBitmap(
            FCM::U_Int32 resourceId,
            DOM::LibraryItem::PIMediaItem pLibItem);

        virtual FCM::Result _FCMCALL AddClassicText(
            FCM::U_Int32 resourceId,
            DOM::FrameElement::PIClassicText pClassicText);

        virtual FCM::Result _FCMCALL HasResource(
            FCM::U_Int32 resourceId,
            FCM::Boolean& hasResource);

        ResourcePalette();

        ~ResourcePalette();

        void Init(OutputWriter* pOutputWriter);

        void Clear();

        FCM::Result HasResource(
            const std::string& name,
            FCM::Boolean& hasResource);

    private:

        FCM::Result ExportFill(DOM::FrameElement::PIShape pIShape);

        FCM::Result ExportStroke(DOM::FrameElement::PIShape pIShape);

        FCM::Result ExportStrokeStyle(FCM::PIFCMUnknown pStrokeStyle);

        FCM::Result ExportSolidStrokeStyle(DOM::StrokeStyle::ISolidStrokeStyle* pSolidStrokeStyle);

        FCM::Result ExportFillStyle(FCM::PIFCMUnknown pFillStyle);

        FCM::Result ExportFillBoundary(DOM::Service::Shape::PIPath pPath);

        FCM::Result ExportHole(DOM::Service::Shape::PIPath pPath);

        FCM::Result ExportPath(DOM::Service::Shape::PIPath pPath);

        FCM::Result ExportSolidFillStyle(
            DOM::FillStyle::ISolidFillStyle* pSolidFillStyle);

        FCM::Result ExportRadialGradientFillStyle(
            DOM::FillStyle::IGradientFillStyle* pGradientFillStyle);

        FCM::Result ExportLinearGradientFillStyle(
            DOM::FillStyle::IGradientFillStyle* pGradientFillStyle);

        FCM::Result ExportBitmapFillStyle(
            DOM::FillStyle::IBitmapFillStyle* pBitmapFillStyle);

        FCM::Result GetFontInfo(DOM::FrameElement::ITextStyle* pTextStyleItem, std::string& name, FCM::U_Int16 fontSize);

        FCM::Result HasFancyStrokes(DOM::FrameElement::PIShape pShape, FCM::Boolean& hasFancy);

        FCM::Result ConvertStrokeToFill(
            DOM::FrameElement::PIShape pShape,
            DOM::FrameElement::PIShape& pNewShape);

    private:

        OutputWriter* m_pOutputWriter;

        std::vector<FCM::U_Int32> m_resourceList;

        std::vector<std::string> m_resourceNames;
    };
}