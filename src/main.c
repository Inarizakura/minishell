/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dphang <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/04 09:06:01 by dphang            #+#    #+#             */
/*   Updated: 2024/04/04 15:02:13 by dphang           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/minishell.h"

int	main(int ac, char **av, char **envp)
{
	(void)ac;
	t_minishell *mnsh;

	init_mnsh(envp, &mnsh);
	//test
	t_envp	*temp;
	temp = mnsh->envp;
	while (temp)
	{
		printf("%s\n", temp->content);
		temp = temp->next;
	}
	//end test
	excu(av + 1, &mnsh);
	//test
	temp = mnsh->envp;
	while (temp)
	{
		printf("%s\n", temp->content);
		temp = temp->next;
	}
	//end test
	return (0);
}
