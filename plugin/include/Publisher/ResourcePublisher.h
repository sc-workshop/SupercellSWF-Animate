#pragma once

#include <FCMTypes.h>
#include <FCMPluginInterface.h>

#include <FillStyle/ISolidFillStyle.h>
#include <FillStyle/IBitmapFillStyle.h>
#include <FrameElement/ITextStyle.h>
#include <Service/Shape/IPath.h>
#include <Utils/DOMTypes.h>

#include "DOM/ILibraryItem.h"
#include "DOM/ILayer2.h"
#include <DOM/LibraryItem/ISymbolItem.h>
#include <DOM/LibraryItem/IMediaItem.h>
#include <DOM/MediaInfo/IBitmapInfo.h>

#include "io/Console.h"
#include "Shared/SharedWriter.h"
#include "FrameGenerator.h"
#include "ShapeGenerator.h"

#include <vector>
#include <string>

using namespace FCM;

namespace DOM
{
    namespace FrameElement
    {
        FORWARD_DECLARE_INTERFACE(IShape);
        FORWARD_DECLARE_INTERFACE(IClassicText);
    }
}

namespace sc {
	namespace Adobe {
		class ResourcePublisher {
            PIFCMCallback m_callback = nullptr;
            SharedWriter* m_writer = nullptr;

            FrameGenerator frameGenerator;
            ShapeGenerator shapeGenerator;

                                    // Name  /  Id
            std::vector<std::pair<std::string, U_Int16>> m_symbolsDict;

            U_Int16 m_id = 0;

            Console console;

        public:
            ResourcePublisher() { }
            ~ResourcePublisher() { }

            Result AddLibraryItem(
                DOM::ILibraryItem* item,
                uint16_t& identifer,
                bool addExportName = false
            );

            Result AddSymbol(
                std::string name,
                DOM::LibraryItem::ISymbolItem* item,
                uint16_t& identifer,
                bool addExportName = false
            );

            Result AddMovieclip(
                std::string name,
                DOM::ITimeline* timeline,
                uint16_t& identifer
            );

            Result AddShape(
                std::string name,
                DOM::ITimeline* timeline,
                uint16_t& identifer
            );

            Result GetIdentifer(
                std::string name,
                uint16_t& identifer
            );

            Result Init(
                SharedWriter* writer,
                PIFCMCallback callback
            );

            Result Finalize();

		};
	}
}