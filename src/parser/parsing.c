/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cwijaya <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/04 15:59:06 by cwijaya           #+#    #+#             */
/*   Updated: 2024/04/10 19:42:56 by cwijaya          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/minishell.h"

int	is_delim(char *c)
{
	return (*c == ' ' || *c == '\t' || *c == '<' || *c == '>' || *c == '|'
		|| *c == '(' || *c == ')' || *c == '\0' || !ft_strncmp(c, "&&", 2));
}

int	loop_quote(char *c)
{
	int		i;
	char	quote;

	quote = *c;
	(c)++;
	i = 1;
	while (*c)
	{
		i++;
		if (*c == quote)
			return (i);
		(c)++;
	}
	return (0);
}

int	ft_isspace(char c)
{
	return (c == ' ' || c == '\t');
}

int	ft_isoperation(char *str)
{
	return (*str == '(' || *str == ')' || *str == '|' || !ft_strncmp(str, "&&",
			2) || !ft_strncmp(str, "||", 2) || *str == '<' || *str == '>'
		|| *str == ';');
}

void	ft_skipspaces(char **str)
{
	while (ft_isspace(**str))
		(*str)++;
}

t_type	get_ops_type(char *ops)
{
	if (!ft_strncmp(ops, "||", 2))
		return (T_OR);
	else if (!ft_strncmp(ops, ">>", 2))
		return (T_APPEND);
	else if (!ft_strncmp(ops, "<<", 2))
		return (T_HEREDOC);
	else if (*ops == '(')
		return (T_OB);
	else if (*ops == ')')
		return (T_CB);
	else if (*ops == '|')
		return (T_PIPE);
	else if (*ops == '<')
		return (T_INPUT);
	else if (*ops == '>')
		return (T_TRUNC);
	else if (*ops == ';')
		return (T_COL);
	else
		return (T_INVALID);
}

char	*ft_strndup(const char *s, int len)
{
	int		i;
	char	*dup;

	i = 0;
	dup = (char *)malloc((len + 1) * sizeof(char));
	if (!dup)
		return (((void *)0));
	while (s[i] && i < len)
	{
		dup[i] = s[i];
		i++;
	}
	dup[i] = '\0';
	return (dup);
}

t_dls	*tokenize_operation(char **input)
{
	t_dls	*token;

	if (!ft_strncmp(*input, "&&", 2) || !ft_strncmp(*input, "||", 2)
		|| !ft_strncmp(*input, ">>", 2) || !ft_strncmp(*input, "<<", 2))
	{
		token = ft_dlsnew(ft_strndup(*input, 2), get_ops_type(*input));
		(*input)++;
	}
	else
		token = ft_dlsnew(ft_strndup(*input, 1), get_ops_type(*input));
	(*input)++;
	return (token);
}

t_dls	*tokenize_param(char **input)
{
	t_dls	*token;
	int		l;

	l = 0;
	if (**input == '"' || **input == '\'')
	{
		l = loop_quote(*input);
		if (l <= 0)
			printf("quote not closed");
	}
	else
	{
		while (!is_delim((*input) + l))
			l++;
	}
	token = ft_dlsnew(ft_strndup(*input, l), T_ARG);
	*input += l;
	return (token);
}

t_dls	*parse_token(char *input)
{
	t_dls	*tokens;
	t_dls	*token;

	tokens = NULL;
	while (*input)
	{
		ft_skipspaces(&input);
		if (get_ops_type(input))
			token = tokenize_operation(&input);
		else
			token = tokenize_param(&input);
		ft_dlsadd_back(&tokens, token);
	}
	return (tokens);
}

int run_builtin(char **av, char **envp)
{
	int exit_status;

	if (!ft_strncmp(av[0], "echo", 5))
		exit_status = echo(av);
	// else if (!ft_strncmp(av[0], "cd"))
	// 	cd(av[1]);
	else if (!ft_strncmp(av[0], "env", 4))
		exit_status = env(av, envp);
	else if (!ft_strncmp(av[0], "pwd", 4))
		pwd();
	// else if (!ft_strncmp(av[0], "export"))
	// 	export(av);
	// else if (!ft_strncmp(av[0], "unset"))
	// 	unset(av);
	return (exit_status);
}

char **parse_to_arg(t_dls *tokens)
{
	t_dls	*tmp;
	char	**av;
	int		i;

	tmp = tokens;
	i = 0;
	while (tmp)
	{
		if (tmp->type == T_ARG)
			i++;
		tmp = tmp->next;
	}
	av = (char **)malloc((i + 1) * sizeof(char *));
	if (!av)
		return (0);
	i = 0;
	tmp = tokens;
	while (tmp)
	{
		if (tmp->type == T_ARG)
		{
			av[i] = ft_strdup(tmp->content);
			i++;
		}
		tmp = tmp->next;
	}
	av[i] = NULL;
	return (av);
}

char *get_dollar(char *str, char **envp)
{
	int		i;
	int		j;
	char	*var;
	int		varlen;

	j = 1;
	while (str[j] && !ft_isspace(str[j]) && !is_delim(&str[j]))
		j++;
	var = ft_strndup(str + 1, j - 1);
	varlen = ft_strlen(var);
	i = 0;
	while (envp[i])
	{
		if (!ft_strncmp(envp[i], var, varlen))
		{
			free(var);
			return (ft_strdup(envp[i] + varlen + 1));
		}
		i++;
	}
	free(var);
	return (str);
}

char **process_av(t_dls *tokens, char **envp)
{
	t_dls	*tmp;
	char	**av;
	char	*dollar_var;
	int		i;

	tmp = tokens;
	i = 0;
	while (tmp)
	{
		if (tmp->content[0] == '$')
		{
			dollar_var = get_dollar(tmp->content, envp);
			free(tmp->content);
			tmp->content = dollar_var;
		}
		//else if for * wild card
		tmp = tmp->next;
	}
	av = parse_to_arg(tokens);
	return (av);
}

void simple_command(char **av, char **envp)
{
	if (is_builtin(av))
		return run_builtin(av, envp);
}

int count_pipe(t_dls *tokens)
{
	int count;

	count = 0;
	while (tokens)
	{
		if (tokens->type == T_PIPE)
			count++;
		tokens = tokens->next;
	}

	return count;
}

t_dls *copy_prev(t_dls **tokens)
{
	t_dls *temp;

	temp = (*tokens)->prev;
	*tokens = (*tokens)->next;
	free((*tokens)->prev);
	(*tokens)->prev = NULL;
	temp->next = NULL;
	while (temp->prev)
	{
		temp = temp->prev;
	}
	return temp;
}

t_dls *copy_next(t_dls **tokens)
{
	*tokens = (*tokens)->next;
	free((*tokens)->prev);
	(*tokens)->prev = NULL;
	return (*tokens);
}

t_type	check_redirect(t_dls *tokens)
{
	while (tokens)
	{
		if (tokens->type >= T_TRUNC)
			return (tokens->type);
		tokens = tokens->next;
	}
	return T_CMD;
}

t_ast **populate_children(t_dls *tokens, int count)
{
	t_ast *ast;
	int i;

	i = 0;
	ast = (t_ast **)malloc(sizeof(t_ast *) * (count + 2));
	while (tokens)
	{
		if (tokens->type == T_PIPE)
		{
			ast->children[i]->tokens = copy_prev(&tokens);
			ast->children[i]->type = check_redirect(ast->children[i]->tokens);
			i++;
		}
		else
			tokens = tokens->next;
	}
	ast->children[i++]->tokens = copy_next(&tokens);
	ast->children[i] = NULL;
}

t_ast *parse_ast(t_dls *tokens)
{
	t_ast *ast;
	int	count;

	ast = (t_ast *)malloc(sizeof(t_ast));
	if (!ast)
		return (1);
	count = count_pipe(tokens);
	if (count > 0)
	{
		ast->type = T_PIPE;
		ast->children = populate_children(tokens, count);
	}
	else
	{
		ast->tokens = tokens;
		ast->type = check_redirect(tokens);
	}
	return ast;
}

int execute_pipe(t_ast **ast)
{
	int fd[2];
	int id;

	if (*ast)
		return 0;
	if (pipe(fd) == -1){
		printf("err");
		return (1);
	}
	id = fork();
	if (id == 0){
		dup2(fd[0], STDIN_FILENO);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[0]);
		close(fd[1]);
		execute_ast((*ast)++);
	}
	close(fd[0]);
	close(fd[1]);
	waitpid(id, NULL, 0);
}

int execute_ast(t_ast *ast)
{
	if (!ast)
		return 0;
	if (ast->type == T_PIPE)
		execute_pipe(ast->children);

	if (ast->type == T_CMD)
		execute_tokens(ast->tokens);
}

int	execute_tokens(t_dls *tokens)
{
	char	**av;
	 char **envp;

	av = process_av(tokens, envp);
	if (!av)
		return (1);
	simple_command(av, envp);

}
