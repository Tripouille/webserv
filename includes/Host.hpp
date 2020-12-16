/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Host.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: frfrey <frfrey@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/12/16 15:08:12 by frfrey            #+#    #+#             */
/*   Updated: 2020/12/16 15:30:52 by frfrey           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef HOST_HPP
# define HOST_HPP

# include <vector>
# include <string>

using std::string;
using std::vector;

class Host
{

	public:

		Host();
		~Host();

		vector<int>			port;
		string				root;
		vector<string>		index;
		string				serverName;
		vector<string>		cgi;

	private:

		Host( Host const & src );
		Host &		operator=( Host const & rhs );
};

#endif /* ************************************************************ HOST_H */
