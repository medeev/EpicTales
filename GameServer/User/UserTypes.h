

#pragma once


class User;

/// 유저 포인터 타입 정의
typedef std::shared_ptr< User >	UserPtr;

/// 유저 위크 포인터 타입 정의
typedef std::weak_ptr< User >   UserWeakPtr;
