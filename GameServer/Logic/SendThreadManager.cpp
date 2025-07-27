

#include "Pch.h"
#include "SendThreadManager.h"

#include <Asio/Service/Connection.h>

#include "Config.h"
#include "Logic/SendThread.h"
#include "User/User.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
SendThreadManager::SendThreadManager()
	:
	_index(0)
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////
SendThreadManager::~SendThreadManager()
{
	Core::Generic::DeleteContainer(_threads);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		초기화한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void SendThreadManager::initialize()
{
	const int32_t thradCount = Config::GetSendThreadCount();
	for (int32_t i = 0; i < thradCount; ++i)
	{
		SendThread* thread = new SendThread();
		thread->start();

		_threads.push_back(thread);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	해제한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void SendThreadManager::finalize()
{
	for (auto thread : _threads)
		thread->stop();

	Core::Generic::DeleteContainer(_threads);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		패킷을 보낸다
////////////////////////////////////////////////////////////////////////////////////////////////////
void SendThreadManager::send(UserPtr userPtr, NaglePacketPackPtr naglePacket)
{
	if (!userPtr)
		return;

	SendThread* sendThread = _threads[(_index++) % Config::GetSendThreadCount()];
	ENSURE(sendThread, return);

	sendThread->getTaskManager().runTask(
		Caller,
		[this, userPtr, naglePacket]()
		{
			bool processed = false;
			if (!naglePacket->processed.compare_exchange_strong(processed, true))
				return WARN_LOG("naglePacket is already processed!!");

			size_t totalSize = naglePacket->totalSize;

			if (
				totalSize <= 0 ||
				totalSize > 1024 * 1024 * 10)
				return WARN_LOG("invalid nagle packet size. [totalSize: %lld]", totalSize);

			naglePacket->totalSize.fetch_sub(totalSize);

			PacketWriter writer;
			writer.resize((uint32_t)totalSize);

			if (!writer.getBuffer())
				return WARN_LOG("packetWriter buffer is null");

			int64_t processedCount = 0;
			int32_t loopCount = 0;
			do
			{
				if (loopCount && !processedCount)
					Core::IThread::Sleep(1);

				if (++loopCount >= 10)
				{
					WARN_LOG(
						"too many loop. [userId: %llu, loopCount: %d, useCount: %d]",
						userPtr->getId(),
						loopCount,
						naglePacket.use_count());

					if (loopCount >= 100)
						break;
				}

				processedCount = naglePacket->queue.consume_all(
					[&writer](NaglePacket* packet)
					{
						const auto& packetWriter = *(packet->writer);
						writer.write(packetWriter.getBuffer(), packetWriter.getLength());
						delete packet;
					});

			} while (processedCount || naglePacket.use_count() != 1);

			userPtr->Core::Connection::send(writer.getBuffer(), writer.getLength());

		});
}
