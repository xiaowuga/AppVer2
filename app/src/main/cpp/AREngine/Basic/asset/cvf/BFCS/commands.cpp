
#include"BFC/commands.h"
#include"BFC/argv.h"

#include"BFC/stdf.h"
//#include<experimental/filesystem>
#include<string.h>

#ifndef _MSC_VER
#define strnicmp strncasecmp
#endif 


_FF_BEG


static FFCommand::CommandList *g_cmdList = NULL;

FFCommand::CommandList::CommandList(const FFCommand::Command *cmds, int count)
	:m_cmds(cmds), m_count(count)
{
	m_pNext = g_cmdList;
	g_cmdList = this;
}

const FFCommand::Command* findCommand(const std::string &cmd) {
	auto *p = g_cmdList;
	while (p)
	{
		for (int i = 0; i < p->m_count; ++i)
		{
			if (stricmp(p->m_cmds[i].m_cmd, cmd.c_str()) == 0)
				return &p->m_cmds[i];
		}
		p = p->m_pNext;
	}
	return nullptr;
}

static void _call_cmd_func(const FFCommand::Command& cmd, ArgSet& args)
{
	if (cmd.m_func)
	{
		if (cmd.m_funcType == FFCommand::FUNCT_APP1)
		{
			typedef void(*FuncT)(ff::ArgSet&);
			FuncT fp = reinterpret_cast<FuncT>(cmd.m_func);

			ff::ArgSet defArgs;
			if (cmd.m_defArgStr)
			{
				//skip leading text for required arguments (i.e. #0 #1 -a -b)
				const char* px = cmd.m_defArgStr;
				while (*px)
				{
					if (*px == '-' && (px == cmd.m_defArgStr || isspace(px[-1])))
						break;
					++px;
				}
				if (*px=='-')
				{
					defArgs.setArgs(px);
					args.setNext(&defArgs);
				}
			}
			fp(args);
		}
		else if (cmd.m_funcType == FFCommand::FUNCT_APP0)
		{
			typedef void(*FuncT)();
			FuncT fp = reinterpret_cast<FuncT>(cmd.m_func);
			fp();
		}
		else
			printf("warning: unknown func-type of command");
	}
}

_BFCS_API void exec(int argc, char* argv[])
{
	if (argc > 1)
	{
		std::string cmdName(argv[1]);
		auto *cmdObj = findCommand(cmdName);
		if (!cmdObj)
		{
			printf("error:can't find command %s\n", cmdName.c_str());
			return;
		}

		if (cmdObj->m_func)
		{
			ff::ArgSet args;
			if (argc > 2)
				args.setArgs(argc - 2, argv + 2, false);

			_call_cmd_func(*cmdObj, args);
		}
	}
}

_BFCS_API void exec(const char *cmd)
{
	if (!cmd)
		return;

	const char *end = cmd + strlen(cmd);
	cmd = ff::SkipSpace(cmd, end);
	const char *px = ff::SkipNonSpace(cmd, end);
	if (cmd != px)
	{
		std::string cmdName(cmd, px);
		auto *cmdObj = findCommand(cmdName);
		if (!cmdObj)
		{
			printf("error:can't find command %s\n", cmdName.c_str());
			return;
		}

		if (cmdObj->m_func)
		{
			ff::ArgSet args;
			args.setArgs(px);
			_call_cmd_func(*cmdObj, args);
		}
	}
}

static void on_list(ArgSet &args)
{
	std::string name = args.getd<string>("#0", "");

	auto *p = g_cmdList;
	while (p)
	{
		for (int i = 0; i < p->m_count; ++i)
		{
			if (name.empty() || strnicmp(p->m_cmds[i].m_cmd, name.c_str(), name.size()) == 0)
			{
				auto &cmd = p->m_cmds[i];
				printf(">%s %s\n %s\n", cmd.m_cmd, cmd.m_defArgStr, cmd.m_shortHelp);
			}
		}
		p = p->m_pNext;
	}
}

static void on_echo(ArgSet& args)
{
	std::string str = args.getd<string>("#0", "");
	printf("%s\n", str.c_str());
}

CMD_BEG()
CMD("list", on_list, "#filter", "list available commands", "")
CMD("echo", on_echo, "#message", "echo a message", "")
CMD_END()

_FF_END

