/*
 * libasiotap - A portable TAP adapter extension for Boost::ASIO.
 * Copyright (C) 2010-2011 Julien KAUFFMANN <julien.kauffmann@freelan.org>
 *
 * This file is part of libasiotap.
 *
 * libasiotap is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * libasiotap is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 *
 * If you intend to use libasiotap in a commercial software, please
 * contact me : we may arrange this for a small fee or no fee at all,
 * depending on the nature of your project.
 */

/**
 * \file base_route_manager.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@freelan.org>
 * \brief The base route manager class.
 */

#ifndef ASIOTAP_BASE_ROUTE_MANAGER_HPP
#define ASIOTAP_BASE_ROUTE_MANAGER_HPP

#include <string>
#include <map>
#include <iostream>

#include <boost/optional.hpp>

#include "types/ip_network_address.hpp"

namespace asiotap
{
	/**
	 * \brief A routing table entry.
	 */
	template <typename InterfaceType>
	struct base_routing_table_entry
	{
		InterfaceType interface;
		ip_network_address network;
		boost::optional<boost::asio::ip::address> gateway;

		friend bool operator==(const base_routing_table_entry& lhs, const base_routing_table_entry& rhs)
		{
			return ((lhs.interface == rhs.interface) && (lhs.network == rhs.network) && (lhs.gateway == rhs.gateway));
		}

		friend bool operator<(const base_routing_table_entry& lhs, const base_routing_table_entry& rhs)
		{
			return ((lhs.interface < rhs.interface) || (lhs.network < rhs.network) || (lhs.gateway < rhs.gateway));
		}

		friend std::ostream& operator<<(std::ostream& os, const base_routing_table_entry& value)
		{
			if (value.gateway)
			{
				return os << value.interface << " - " << value.network << " - " << *(value.gateway);
			}
			else
			{
				return os << value.interface << " - " << value.network << " - no gateway";
			}
		}
	};

	/**
	 * \brief Handle system routes.
	 */
	template <typename RouteManagerType, typename RouteType>
	class base_route_manager
	{
		public:
			typedef RouteType route_type;

			base_route_manager() = default;

			base_route_manager(const base_route_manager&) = delete;
			base_route_manager& operator=(const base_route_manager&) = delete;

			base_route_manager(base_route_manager&&) = delete;
			base_route_manager& operator=(base_route_manager&&) = delete;

			~base_route_manager()
			{
				for (auto&& route : m_routing_table)
				{
					try
					{
						static_cast<RouteManagerType*>(this)->unregister_route(route.first);
					}
					catch (boost::system::system_error&)
					{
						// We don't care about errors at this point: we can't handle them and we must continue anyway.
					}
				}
			}

			void has_route(const route_type& route)
			{
				return (m_routing_table.find(route) != m_routing_table.end());
			}

			bool add_route(const route_type& route)
			{
				if (m_routing_table[route]++ == 0)
				{
					static_cast<RouteManagerType*>(this)->register_route(route);

					return true;
				}

				return false;
			}

			bool remove_route(const route_type& route)
			{
				const auto route_position = m_routing_table.find(route);

				if (route_position != m_routing_table.end())
				{
					if (route_position->second == 1)
					{
						static_cast<RouteManagerType*>(this)->unregister_route(route);

						m_routing_table.erase(route_position);

						return true;
					}
					else if (route_position->second > 1)
					{
						--route_position->second;
					}
				}

				return false;
			}

		protected:

			typedef std::map<route_type, unsigned int> routing_table_type;

		private:

			routing_table_type m_routing_table;
	};
}

#endif /* ASIOTAP_BASE_ROUTE_MANAGER_HPP */
