#include <stdio.h>
#include <thread>
#include <functional>
#include <queue>

int g_cooked1 = 0;   // 요리가 끝났는가?
int g_cooked2 = 0;
int g_cooked3 = 0;

int g_exit = 0;

void Beeper(int num)
{
	switch (num)
	{
	case 1:
		printf("1번 누룽지 삼계탕 완성~\n");
		g_cooked1 = 1;
		break;
	case 2:
		printf("2번 누룽지 삼계탕 완성~\n");
		g_cooked2 = 1;
		break;
	case 3:
		printf("3번 누룽지 삼계탕 완성~\n");
		g_cooked3 = 1;
		break;
	}
}

std::queue<std::function<void()>> orderque;

void Order(std::function<void()> cb, int num)
{
	switch (num)
	{
	case 1:
		orderque.push(cb);
		printf("네~ 1번이요\n");
		break;
	case 2:
		orderque.push(cb);
		printf("네~ 2번이요\n");
		break;
	case 3:
		orderque.push(cb);
		printf("네~ 3번이요\n");
		break;
	}
}

void Manager_Update(void)
{
	if (!orderque.empty())
	{
		auto cb = orderque.front();
		orderque.pop();
		cb();
	}
		
}

void RunLoop(void)
{
	while (!g_exit)
	{
		Manager_Update();

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

void Receiving()
{
	while (1)
	{
		int b;
		scanf_s(" %d", &b, sizeof(b));

		switch (b)
		{
		case 1:
			printf("1번 주문이요~\n");
			Order([]() { Beeper(1); }, 1);
			break;
		case 2:
			printf("2번 주문이요~\n");
			Order([]() { Beeper(2); }, 2);
			break;
		case 3:
			printf("3번 주문이요~\n");
			Order([]() { Beeper(3); }, 3);
			break;
		case 4:
			printf("다 나왔슈~\n");
			g_exit = 1;
			return;

		}
	}
	
}


int main(void)
{
	printf("주문함\n");

	std::thread t1(RunLoop);
	std::thread t2(Receiving);
	t1.join();
	t2.join();

	
	printf("식사 시작\n");
	return 0;
}