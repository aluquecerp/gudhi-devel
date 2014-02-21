/*
 * First_vertex_placement.h
 *
 *  Created on: Feb 20, 2014 
 *      Author: David Salinas
 *  Copyright 2013 INRIA. All rights reserved
 */

#ifndef FIRST_VERTEX_PLACEMENT_H_
#define FIRST_VERTEX_PLACEMENT_H_


namespace contraction {


/**
 * @brief Places the contracted point onto the first point of the edge
 */
template< typename EdgeProfile> class First_vertex_placement : public Placement_policy<EdgeProfile>{

public:
	typedef typename EdgeProfile::Point Point;
	typedef typename EdgeProfile::edge_descriptor edge_descriptor;
	typedef typename EdgeProfile::Vertex Vertex;

	typedef typename Placement_policy<EdgeProfile>::Placement_type Placement_type;

	Placement_type operator()(const EdgeProfile& profile){
		return Placement_type(profile.p0());
	}
};
}  // namespace contraction




#endif /* FIRST_VERTEX_PLACEMENT_H_ */
