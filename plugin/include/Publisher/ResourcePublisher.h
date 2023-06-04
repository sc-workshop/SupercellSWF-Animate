#pragma once

#include <vector>
#include <string>

#include "io/Console.h"

// FCM stuff
#include <FCMTypes.h>

// Timeline
#include <DOM/ILayer2.h>

// Symbol
#include <DOM/ILibraryItem.h>
#include <DOM/LibraryItem/ISymbolItem.h>
#include <DOM/LibraryItem/IMediaItem.h>

// Image processing
#include <opencv2/opencv.hpp>

// Modifier
#include <SupercellFlash/objects/MovieClipModifier.h>

// Generators
#include "Publisher/ShapeGenerator.h"
#include "Publisher/TimelineBuilder/Builder.h"

// Writer
#include "Publisher/Shared/SharedWriter.h"

using namespace FCM;
using namespace std;

namespace sc {
	namespace Adobe {
		class ResourcePublisher {
            PIFCMCallback m_callback;
            SharedWriter* m_writer;

            TimelineBuilder timelineBuilder;
            ShapeGenerator shapeGenerator;

                                    // Name  /  Id
            vector<pair<u16string, uint16_t>> m_symbolsDict;
                                            // Type / Id
            vector< pair<sc::MovieClipModifier::Type, uint16_t>> m_modifierDict;

            uint32_t m_id = 0;

            Console console;

            // Services
            AutoPtr<IFCMCalloc> m_calloc;
            AutoPtr<DOM::Service::Tween::ITweenerService> m_tweener;

        public:
            ResourcePublisher(PIFCMCallback callback, SharedWriter* writer):
                m_callback(callback),
                timelineBuilder(callback, *this),
                shapeGenerator(callback, *this),
                m_writer(writer)
            {
                console.Init("ResourcePublisher", m_callback);
            }

            ~ResourcePublisher() { }

            uint16_t AddLibraryItem(
                DOM::ILibraryItem* item,
                bool hasName = false
            );

            uint16_t AddSymbol(
                u16string name,
                DOM::LibraryItem::ISymbolItem* item,
                bool hasName = false
            );

            uint16_t AddMovieclip(
                u16string name,
                AutoPtr<DOM::ITimeline> timeline,
                bool hasName
            );

            uint16_t AddShape(
                u16string name,
                DOM::ITimeline* timeline
            );

            uint16_t AddModifier(
                sc::MovieClipModifier::Type type
            );

            uint16_t GetIdentifer(
                u16string name
            );

            uint16_t GetIdentifer(
                sc::MovieClipModifier::Type type
            );

            void Finalize();

            // Services
            AutoPtr<DOM::Service::Tween::ITweenerService> GetTweenerService() {
                if (!m_tweener) {
                    FCM::AutoPtr<FCM::IFCMUnknown> unk;
                    m_callback->GetService(TWEENER_SERVICE, unk.m_Ptr);

                    m_tweener = unk;
                }

                return m_tweener;
            }

            AutoPtr<IFCMCalloc> GetCallocService() {
                if (!m_calloc) {
                    m_calloc = Utils::GetCallocService(m_callback);
                }

                return m_calloc;
            }
		};
	}
}