////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		룸 타입 정의
////////////////////////////////////////////////////////////////////////////////////////////////////


class Room;
typedef std::shared_ptr<Room> RoomPtr;
typedef std::weak_ptr<Room> RoomWeakPtr;

class World;
typedef std::shared_ptr<World> WorldPtr;

class DungeonWorld;
typedef std::shared_ptr<DungeonWorld> DungeonWorldPtr;

class UserWaitRoom;
typedef std::shared_ptr<UserWaitRoom> UserWaitRoomPtr;
typedef std::weak_ptr<UserWaitRoom> UserWaitRoomWeakPtr;

class InstanceQuestWorld;
typedef std::shared_ptr<InstanceQuestWorld> InstanceQuestWorldPtr;
